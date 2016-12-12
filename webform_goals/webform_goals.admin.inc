<?php

/**
 * @file Admin screens.
 */

function webform_goals_admin_list($form, $form_state = array()) {
  $goals = webform_goal_load_multiple();
  $rows = array();
  foreach ($goals as $goal) {
    // get progress
    $selected_context = _webform_goals_get_selected_context($form_state, $goal);
    $selected_metric = _webform_goals_get_selected_metric($selected_context, $form_state, $goal);
    $data = webform_goals_track_metric($goal);

    if ($selected_context['id'] == 'single') {
      $node = node_load($goal['extra']['selected_form_id']);
      // Do not display data for this goal if its node does not exist:
      if (!$node) {
        continue;
      }
      $selected_context['name'] .= ' : ' . l($node->title, 'node/' . $node->nid);
    }
    elseif ($selected_context['id'] == 'group') {
      $selected_context['name'] .= ' : ' . $goal['extra']['selected_node_type'];
    }
    $row = array();
    $row[] = $goal['name'];
    $row[] = $selected_context['name'];
    $row[] = $selected_metric['name'];
    // @todo for single context add link to parent node or node title
    $row[] = $goal['target_value'];
    // @todo amount: circle back once metrics tracking code is in play.
    $row[] = $data['count'];
    $row[] = $data['percent'] . '%';
    $row[] = webform_goals_goal_status($goal);
    $row[] = _webform_goals_actions($goal);
    $rows[] = $row;
  }

  /*
  $form['goal_filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filters'),
  );*/
  // TODO: add filtering?
  $header = array(
    t('Name'),
    t('Context'),
    t('Metric'),
    t('Goal'),
    t('Amount'),
    t('Progress'),
    t('Status'),
    t('Actions'),
  );
  $vars = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(),
    'colgroups' => array(),
    'caption' => '',
    'sticky' => FALSE,
    'empty' => '',
  );

  $form['goal_list'] = array(
    '#type' => 'markup',
    '#markup' => theme_table($vars),
  );
  $form['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add a goal'),
  );
  $form['#submit'][] = 'webform_goals_admin_redirect';
  return $form;
}

function webform_goals_admin_redirect($form, $form_state) {
  drupal_goto('admin/config/content/webform_goals/add');
}


function _webform_goals_actions($goal) {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  $output = '';
  $links[] = l(t('Edit'), 'admin/config/content/webform_goals/' . $goal['gid'] . '/edit');
  $links[] = l(t('Delete'), 'admin/config/content/webform_goals/' . $goal['gid'] . '/delete');
  $options = array(
    'attributes' => array(
      'class' => array('ctools-use-modal ctools-modal-modal-popup-small'),
      'html' => TRUE,
    ),
  );
  $links[] = l(t('Embed code'), 'webform_goals/' . $goal['gid'] . '/embed', $options);

  foreach ($links as $link) {
    // lame
    $output .= $link . '&nbsp;&nbsp;';
  }
  return $output;
}

function webform_goals_admin_settings($form, $form_state) {
  $default_template = theme('webform_goals_default_template', array());
  $form['cron'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update settings'),
  );
  $form['cron']['webform_goals_standalone_cron'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable standalone cron'),
    '#description' => t('When standalone cron is enabled, maintenance tasks like updating goal progress will no longer be performed during hook_cron and will need to be invoked via /webform_goals_cron. This setting is useful for sites with busy cron tabs as it permits running goal updates as a separate process'),
    '#default_value' => variable_get('webform_goals_standalone_cron', 0),
  );
  $form['cron']['webform_goals_widget_update_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Goal update maximum'),
    '#description' => t('Calculating goal progress can be system intensive for large group goals. If goal progress is not updating in a reasonable timeframe try lowering this number.'),
    '#default_value' => variable_get('webform_goals_widget_update_limit', '50'),
  );

  $form['loader_file'] = array(
    '#type' => 'fieldset',
    '#title' => t('Loader file'),
  );

  $form['loader_file']['rebuild_loader_file'] = array(
    '#type' => 'submit',
    '#value' => t('Rebuild loader file'),
    '#submit' => array('webform_goals_rebuild_loader_file_submit'),
  );

  $form['improved_multi_select'] = array(
    '#type' => 'fieldset',
    '#title' => t('Improved Multi Select'),
    '#description' => t('The Springboard Improved Multi Select module can be used to enhance the user experience in webform goals when building form groups.  It is not required however.'),
  );


  $improved_multi_select_status = module_exists('improved_multi_select') ? 'Enabled' : 'Not enabled';
  $springboard_improved_multi_select_status = module_exists('springboard_improved_multi_select') ? 'Enabled' : 'Not enabled';
  $items = array(
    'Improved Multi Select module status: ' . $improved_multi_select_status,
    'Springboard Improved Multi Select module status: ' . $springboard_improved_multi_select_status,
  );

  $form['improved_multi_select']['status_list'] = array(
    '#theme' => 'item_list',
    '#items' => $items,
  );

  if (module_exists('improved_multi_select')) {
    if (module_exists('springboard_improved_multi_select')) {
      $improved_multi_select_text = 'Both modules are correctly enabled. ' . l('Configure them here', 'admin/config/user-interface/ims') . ' by adding the path admin/config/content/webform_goals/* if it is not already one of the enabled paths.';
    }
    else {
      $improved_multi_select_text = 'Improved Multi Select module is enabled, but Springboard Improved Multi Select is not enabled.  Please enable that as well. ';
    }
  }
  else {
    $improved_multi_select_text = 'Enable both modules if you want the additional functionality.';
  }

  $form['improved_multi_select']['module_status'] = array(
    '#type' => 'markup',
    '#markup' => $improved_multi_select_text,
  );


  return system_settings_form($form);
}

function webform_goals_rebuild_loader_file_submit($form, &$form_state) {
  if (webform_goals_save_loader_file()) {
    drupal_set_message('The loader file has been successfully created.');
  }
  else {
    drupal_set_message('Unable to create the loader file.');
  }
}
