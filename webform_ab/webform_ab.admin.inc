<?php
/**
 * @file
 * Multivariate Testing admin functions
 */

/**
 * Menu callback, admin form.
 */
function webform_ab_admin_settings($form, &$form_state) {
  $form = array();
  $form['webform_ab_ignore_hits_from_roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles to ignore hits from'),
    '#description' => t('Hits and conversions will be ignored from users of the chosen roles.'),
    '#options' => user_roles(),
    '#default_value' => variable_get('webform_ab_ignore_hits_from_roles', array()),
  );
  $form['webform_ab_ignore_hits_from_ips'] = array(
    '#type' => 'textarea',
    '#title' => t('IPs to ignore hits from'),
    '#description' => t('Ignore webform hits and conversions from users in the listed IPs. List one IP per row.'),
    '#cols' => 60,
    '#rows' => 5,
    '#default_value' => variable_get('webform_ab_ignore_hits_from_ips', ''),
  );
  $form['webform_ab_ignore_repeat_hits'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log only one hit per user per form'),
    '#description' => t('If checked, hits on a webform will only be logged once per user.'),
    '#default_value' => variable_get('webform_ab_ignore_repeat_hits', '1'),
  );
  $form['webform_ab_ignore_search_bot_hits'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ignore hits from search engine bots'),
    '#description' => t('If checked, hits from search engines like Google will be ignored.'),
    '#default_value' => variable_get('webform_ab_ignore_search_bot_hits', '1'),
  );
  return system_settings_form($form);
}

/**
 * Menu callback for the Reports and Monitoring tab on Multivariate Test
 *
 * @param $node Node object
 */
function webform_ab_test_reports($node) {
  if (empty($node->included_webforms) || !is_array($node->included_webforms)) {
    return t('No webforms have been added to this Multivariate test. Visit the !forms_tab tab to add webforms.',
        array('!forms_tab' => l(t('Included Webforms'), 'node/' . $node->nid . '/webforms')));
  }

  $return = '';

  // Status labels
  $status_labels = array(
    WEBFORM_AB_WEBFORM_STATUS_ACTIVE => t('Active'),
    WEBFORM_AB_WEBFORM_STATUS_WINNER => t('Winner'),
    WEBFORM_AB_WEBFORM_STATUS_RETIRED => t('Retired'),
  );

  // Set headers present for all Multivariate reports:
  $headers = array(
    t('Status'),
    t('Name'),
  );

  // Add columns for the currently enabled win conditions and gather progress data:
  $enabled_win_conditions = array();
  $all_win_conditions = _webform_ab_get_win_condition_plugins();
  $enabled_win_conditions_query = db_query("SELECT plugin_key, options FROM {webform_ab_win_conditions} " .
    "WHERE test_nid = :webform_ab_nid", array(':webform_ab_nid' => $node->nid));
  foreach ($enabled_win_conditions_query as $condition_data) {
    $plugin_options = unserialize($condition_data->options);
    $enabled_win_conditions[$condition_data->plugin_key]['value_to_win'] = array_pop($plugin_options);
    $headers[] = $all_win_conditions[$condition_data->plugin_key]['title'];
  }

  // Loop over this test's webforms, adding a row for each webform being tested:
  foreach ($node->included_webforms as $webform_nid => $webform_details) {
    $line = array('status' => 'N/A', 'title' => 'N/A');

    // Get this test's webform_ab status and node title:
    if (isset($webform_details['status']) && isset($status_labels[$webform_details['status']])) {
      $line['status'] = $status_labels[$webform_details['status']];
    }
    if ($internal_name = _webform_ab_get_internal_name($webform_details['webform_nid'])) {
      $title_value = $internal_name;
    }
    else {
      $title_value = $webform_details['title'];
    }
    $line['title'] = l($title_value, 'node/' . $webform_details['webform_nid']);

    // Add values for each win condition column:
    foreach ($enabled_win_conditions as $plugin_key => $condition_data) {
      $condition_current_value = '?';
      $current_value_function = ctools_plugin_get_function($all_win_conditions[$plugin_key], 'current value');
      if (is_string($current_value_function)) {
        $current_value = $current_value_function($webform_nid, $webform_details, $node);
      } 
      else {
        // Output an error value if this win condition plugin is missing its "current value" function:
        $current_value = t('Plugin Error');
        drupal_set_message(t('The win condition plugin, ":plugin_key" is missing its "current value" function. Please try clearing cache.',
          array(':plugin_key' => $plugin_key)), 'error');
      }
      // Account for values that should be displayed in percentage format:
      if (stripos($plugin_key, 'percent') !== FALSE) {
        $line[$plugin_key] = ($current_value * 100) . '% ' . t('of') . ' ' . ($condition_data['value_to_win'] * 100) . '%';
      }
      // Account for boolean values:
      elseif (in_array($plugin_key, array('donations_highest_avg', 'donations_highest_gross'))) {
        $line[$plugin_key] = $current_value ? 'Yes' : 'No';
      }
      else {  
        $line[$plugin_key] = $current_value . ' ' . t('of') . ' ' . $condition_data['value_to_win'];
      }
    }
    $rows[] = $line;
  }

  $return .= theme('table', array('header' => $headers, 'rows' => $rows));
  return $return;
}

/**
 * Menu callback for the Included Forms tab on Multivariate test nodes
 *
 * @param $node Node object
 */
function webform_ab_test_webforms($node) {
  $return = '';

  // Status labels
  $status_labels = array(
    WEBFORM_AB_WEBFORM_STATUS_ACTIVE => t('Active'),
    WEBFORM_AB_WEBFORM_STATUS_WINNER => t('Winner'),
    WEBFORM_AB_WEBFORM_STATUS_RETIRED => t('Retired'),
  );

  // Show the list of currently-included forms
  $headers = array(
    t('Name'),
    t('Status'),
    t('Operations'),
  );

  // Prevent removal of tests if any tests have recorded data already:
  $testing_has_started = _webform_ab_has_testing_started($node);
  $rows = array();
  if (!empty($node->included_webforms) && is_array($node->included_webforms)) {
    foreach ($node->included_webforms as $webform_nid => $webform_details) {
      $webform_node = node_load($webform_nid);
      if ($internal_name = _webform_ab_get_internal_name($webform_details['webform_nid'])) {
        $title = $internal_name;
      }
      else {
        $title = $webform_details['title'];
      }
      $removal_link = l(t('remove from test'), 'node/' . $node->nid . '/webforms/remove/' . $webform_nid);
      $rows[] = array(
        l($title, 'node/' . $webform_nid),
        $status_labels[$webform_details['status']],
        l(t('view'), 'node/' . $webform_nid) . ' ' .
        l(t('edit'), 'node/' . $webform_nid . '/edit') . ' ' .
        ($testing_has_started ? '&nbsp;' : $removal_link),
      );
    }
  }

  // If there are no webforms included, show an empty message
  if (count($rows) == 0) {
    $rows[] = array(array(
        'data' => t('No webforms have been added to this Multivariate test. ' .
          'Select a webform in the dropdown below and click the Add button.'),
        'colspan' => count($headers),
      ));
  }
  $return .= theme('table', array('header' => $headers, 'rows' => $rows));
  // Show a form to add more
  $form = drupal_get_form('webform_ab_webform_list_add', $node);
  $return .= drupal_render($form);
  return $return;
}

/**
 * Form to add webforms to the Multivariate Test
 *
 * @param $form_state
 * @param $node Node object
 */
function webform_ab_webform_list_add($form, &$form_state, $node) {
  // Get all valid forms for the form type from other modules
  $webform_types = isset($node->webform_types) ? $node->webform_types : array();
  $flat_forms = module_invoke_all('webform_ab_valid_webforms', $webform_types);
  // Since module_invoke_all() will wipe out numeric array indices, the
  // implementations of hook_webform_ab_valid_webforms() have to return the nid
  // and title within a sub-array, rather than using the nid as an array key.
  // Re-index the array to use the nid as the key and title as the value.
  $indexed_forms = array();
  foreach ($flat_forms as $form) {
    $form = (array) $form;
    // Don't add a webform in the list if it's already included in the test
    if (!isset($node->included_webforms[$form['nid']])) {
      $indexed_forms[$form['nid']] = $form['title'];
    }
  }
  // Allow modules to alter the list of forms
  drupal_alter('webform_ab_valid_webforms', $indexed_forms, $node->webform_types);

  $form = array();
  $form['add_form_id'] = array(
    '#type' => 'select',
    '#title' => t('Add webform'),
    '#description' => t('Choose a webform to add to this test'),
    '#options' => array(0 => t('-- Select one --')) + $indexed_forms,
    '#default_value' => -1,
  );
  $form['test_nid'] = array(
    '#type' => 'hidden',
    '#value' => $node->nid,
  );
  $form['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  // If there are no more webforms that may be added or testing has started, disable the form
  if (count($indexed_forms) == 0 || _webform_ab_has_testing_started($node)) {
    unset($form['add']);
    $form['add_form_id']['#title'] = '';
    $form['add_form_id']['#disabled'] = TRUE;
    $form['add_form_id']['#type'] = 'fieldset';
    if (count($indexed_forms) == 0) {
      $disabled_descr = t('There are no more webforms available to be added to this Multivariate test.');
    }
    else {
      $disabled_descr = t('Webforms cannot be added or removed because testing has started. Click "Reset Data" to add or remove tests.');
    }
    $form['add_form_id']['#description'] = $disabled_descr;
  }

  return $form;
}

/**
 * Handle submission of the form to add a webform to an Multivariate Test
 */
function webform_ab_webform_list_add_submit($form, &$form_state) {
  // Only continue if they actually selected a webform
  if ($form_state['values']['add_form_id'] > 0) {
    // Make sure the chosen webform isn't already in the test
    // It wouldn't have been an option if it was already in the test, but be on
    // the safe side
    $exists = (bool) db_query('SELECT 1 FROM {webform_ab_forms} ' .
      'WHERE test_nid = :test_nid AND webform_nid = :webform_nid',
      array(
        ':test_nid' => $form_state['values']['test_nid'],
        ':webform_nid' => $form_state['values']['add_form_id']
      ))->fetchField() > 0;
    if (!$exists) {
      $record = array(
        'test_nid' => $form_state['values']['test_nid'],
        'webform_nid' => $form_state['values']['add_form_id'],
        'status' => WEBFORM_AB_WEBFORM_STATUS_ACTIVE,
      );
      drupal_write_record('webform_ab_forms', $record);
    }
  }
}

/**
 * Show a confirmation form when a user tries to remove a webform from a test
 */
function webform_ab_test_remove_webform($node, $webform_nid) {
  // Make sure the webform they want to remove is actually in the test
  $form_is_included = (bool) db_query('SELECT 1 FROM {webform_ab_forms} ' .
    'WHERE test_nid = :test_nid AND webform_nid = :webform_nid',
    array(':test_nid' => $node->nid, ':webform_nid' => $webform_nid))->fetchField() > 0;
  if ($form_is_included) {
    // Show a confirmation form
    $form = drupal_get_form('webform_ab_test_remove_webform_confirm', $node->nid, $webform_nid);
    return drupal_render($form);
  }
  else {
    // Somehow tried to remove a webform that isn't in the test
    drupal_set_message(t('That form is not in this Multivariate test.'), 'error');
    drupal_goto('node/' . $node->nid . '/webforms');
  }
}

/**
 * Confirmation form when removing a webform from a test
 */
function webform_ab_test_remove_webform_confirm($form, &$form_state, $test_nid, $webform_nid) {
  $form = array();
  $form['#test_nid'] = $test_nid;
  $form['#webform_nid'] = $webform_nid;
  $title = db_query('SELECT n.title from {node} n WHERE n.nid = :nid',
    array(':nid' => $webform_nid))->fetchField();
  return confirm_form(
    $form,
    t('Are you sure you want to remove webform %title from the Multivariate test?', array('%title' => $title)),
    'node/' . $test_nid . '/webforms',
    t('This action cannot be undone.'),
    t('Remove'),
    t('Cancel'),
    'webform_ab_test_remove_webform_confirm');
}

/**
 * Remove a webform from a test
 */
function webform_ab_test_remove_webform_confirm_submit($form, &$form_state) {
  // Remove data specific to the form that is being removed from this Multivariate test:
  $tables_to_update = array(
    'webform_ab_forms',
    'webform_ab_conversion',
    'webform_ab_hits',
  );
  foreach ($tables_to_update as $table_name) {
    db_delete($table_name)
      ->condition('test_nid', $form['#test_nid'])
      ->condition('webform_nid', $form['#webform_nid'])
      ->execute();
  }

  // Notify other modules that the webform is being removed
  module_invoke_all('webform_ab_remove_form', $form['#test_nid'], $form['#webform_nid']);
  drupal_set_message(t('The form has been removed from the Multivariate test.'));
  $form_state['redirect'] = 'node/' . $form['#test_nid'] . '/webforms';
}

/**
 * Show a confirmation form when a user tries to clear data on a test
 */
function webform_ab_clear_test_data_confirm($form, &$form_state, $node) {
  $form = array();
  $form['#test_nid'] = $node->nid;
  return confirm_form(
    $form,
    t('Are you sure you want to clear all hit and conversion data on the test %title?', array('%title' => $node->title)),
      'node/' . $node->nid . '/reports',
    t('This action will remove all hit and conversion records saved for this test, return all included webforms to ' .
      'the Active status, and it cannot be undone. Note that any data submitted to the included webforms will remain ' .
      'intact, it just won\'t be associated with this MultivariateTest or used to declare a winning webform.'),
    t('Reset'),
    t('Cancel'));
}

/**
 * Clear the data in the test. Removes hits, conversions, and returns all webforms to
 * the active status
 */
function webform_ab_clear_test_data_confirm_submit($form, &$form_state) {
  db_delete('webform_ab_conversion')
  ->condition('test_nid', $form['#test_nid'])
  ->execute();
  db_delete('webform_ab_hits')
  ->condition('test_nid', $form['#test_nid'])
  ->execute();
  db_update('webform_ab_forms')
  ->fields(array(
    'status' => WEBFORM_AB_WEBFORM_STATUS_ACTIVE,
    'win_time' => 0,
  ))
  ->condition('test_nid', $form['#test_nid'])
  ->execute();
  // Notify other modules that may have test data to wipe
  module_invoke_all('webform_ab_reset_data', $form['#test_nid']);
  drupal_set_message(t('The data has been cleared.'));
  $form_state['redirect'] = 'node/' . $form['#test_nid'] . '/reports';
}

/**
 * Get the internal name for a given webform, which is located within
 * a node field.  Return FALSE if no internal name is found.
 * @param $nid
 *   The node ID of the webform in question.
 */
function _webform_ab_get_internal_name($nid) {
  $node_type = db_query("SELECT type FROM {node} WHERE nid = :nid", array(':nid' => $nid))
    ->fetchField();
  if (!$node_type) {
    return FALSE;
  }
  $field_info = field_info_instances('node', $node_type);
  foreach ($field_info as $field_name => $data) {
    if (strpos($field_name, 'internal_name') !== FALSE) {
      $db_table = 'field_data_' . $field_name; 
      $value_column = $field_name . '_value';
    }
  }
  // Get the internal mame, if possible:
  return $internal_name_query = db_query("SELECT $value_column FROM {$db_table} WHERE entity_id = :nid",
    array(':nid' => $nid))
    ->fetchField();
}

/**
 * Returns TRUE if any webform_ab tests are populated with data.
 *
 * @param object $test_node
 *   Node Object of the a/b test node.
 *
 * @return bool
 *   TRUE or FALSE whether testing ahs started.
 *
 */
function _webform_ab_has_testing_started($test_node) {

  $hits = db_query('SELECT COUNT(hit_id) as hits FROM {webform_ab_hits} WHERE test_nid = :test_nid ', array(':test_nid' => $test_node->nid))->fetchField();

  if (!$hits) {
    // Test hasn't started yet.
    return;
  }

  $enabled_win_conditions = array();
  $enabled_win_conditions_query = db_query("SELECT plugin_key FROM {webform_ab_win_conditions} " .
    "WHERE test_nid = :webform_ab_nid", array(':webform_ab_nid' => $test_node->nid));
  foreach ($enabled_win_conditions_query as $condition_data) {
    $enabled_win_conditions[] = $condition_data->plugin_key;;
  }
  $win_conditions = _webform_ab_get_win_condition_plugins();
  foreach ($win_conditions as $plugin_key => $plugin) {

    if (!in_array($plugin_key, $enabled_win_conditions)) {
      continue;
    }

    if ($has_ab_test_started_function = ctools_plugin_get_function($plugin, 'current value')) {

      foreach ($test_node->included_webforms as $webform_nid => $data) {
        if (function_exists($has_ab_test_started_function)) {
          if ($has_ab_test_started_function($webform_nid, array(), $test_node)) {
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}
