<?php

/**
 * @file
 * Admin functions for peer to peer.
 */

/**
 * Page callback for the p2p dashboard.
 */
function springboard_p2p_dashboard() {

  if (springboard_p2p_no_content_types_enabled()) {
    drupal_set_message('No content types have been enabled for peer to peer.  Please visit the settings and configure them.', 'error');
  }
  
  $views = array(
    'p2p_campaigns' => array (
      'header' => t('Active peer to peer campaigns'),
      'content' => views_embed_view('p2p_campaigns', 'block'),
      'create_link' => array('title' => t('Create a new campaign'), 'href' => 'node/add/p2p-campaign'),
      'viewmore_link' => array('title' => t('See all campaigns'), 'href' => 'admin/springboard/p2p/campaign')
    ),
    'p2p_categories' => array(
      'header' => t('Campaign categories'),
      'content' => views_embed_view('p2p_categories', 'block'),
      'create_link' => array('title' => t('Create a new category'), 'href' => 'node/add/p2p-category'),
      'viewmore_link' => array('title' => t('See all categories'), 'href' => 'admin/springboard/p2p/category'),
    ),
    'p2p_user_approval' => array(
      'header' => t('Users awaiting approval'),
      'content' => views_embed_view('p2p_user_approval', 'dashboard_page'),
      'viewmore_link' => array('title' => t('Open approval queue'), 'href' => 'admin/springboard/p2p/approval/user'),
    ),
    'p2p_personal_campaigns_admin' => array(
      'header' => t('Personal campaigns'),
      'content' => views_embed_view('p2p_personal_campaigns_admin', 'block_1'),
      'viewmore_link' => array('title' => t('View all personal campaigns'), 'href' => 'admin/springboard/p2p/personal-campaigns'),
    ),
    'p2p_landing_pages' => array(
      'header' => t('Landing pages'),
      'content' => views_embed_view('p2p_landing_pages', 'block'),
      'create_link' => array('title' => t('Create a new landing page'), 'href' => 'node/add/p2p-campaign-landing'),
      'viewmore_link' => array('title' => t('View all landing pages'), 'href' => 'admin/springboard/p2p/landing-pages'),
    ),
  );
  
  return theme('springboard_p2p_dashboard', array('views' => $views));

}

/**
 * Form callback for the p2p global admin settings.
 */
function springboard_p2p_admin_settings() {
  $form = array();

  $form['help_message'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messages'),
    '#weight' => 225,
  );

  // Reset aliases
  $form['url_aliases'] = array(
    '#type' => 'fieldset',
    '#title' => t('Reset Aliases to Defaults'),
    '#weight' => -50,
    '#description' => '<p>' . t('Be very careful with this; it will reset the URL patterns for all Peer-to-Peer content types.') . '</p>',
  );
  $form['url_aliases']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => array('springboard_p2p_reset_url_aliases'),
  );

  $description = 'This message displays on the Peer to Peer registration page when someone tries to register with an email that matches an existing account. To edit the email that the user receives, please !rules.';
  $replacements = array(
    '!rules' => l('go here', 'springboard/p2p/rules'),
  );

  $form['help_message']['existing_user_message_area'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#title' => t('Existing User Account Notification Message'),
    '#required' => TRUE,
    '#rows' => 2,
    '#default_value' => variable_get('springboard_p2p_existing_user_message_area', ''),
    '#description' => t($description, $replacements),
  );

  // Login message text area.
  // We set a var here, otherwise drupal complains about htmlspecialchars.
  // This is a known bug with text_format FAPI.
  $textformat = variable_get('springboard_p2p_login_message_area', array('value' => '', 'format' => NULL));
  // Now render the form item.
  $form['help_message']['login_message_area'] = array(
    '#type' => 'text_format',
    '#title' => t('P2P Login message'),
    '#required' => TRUE,
    '#rows' => 5,
    '#format' => $textformat['format'],
    '#default_value' => $textformat['value'],
    '#description' => t('Use this area to set a P2P Login message that will appear on the registration page for your users located at /p2p/register.'),
  );

  // Help message text area.
  // We set a var here, otherwise drupal complains about htmlspecialchars.
  // This is a known bug with text_format FAPI.
  $textformat = variable_get('springboard_p2p_help_message_area', array('value' => '', 'format' => NULL));
  // Now render the form item.
  $form['help_message']['help_message_area'] = array(
    '#type' => 'text_format',
    '#title' => t('P2P help message'),
    '#required' => TRUE,
    '#rows' => 5,
    '#format' => $textformat['format'],
    '#default_value' => $textformat['value'],
    '#description' => t('This creates a block with a help message.'),
  );

  $form['form_types'] = array(
    '#type' => 'fieldset',
    '#title' => 'Peer to peer enabled form types',
    '#weight' => -40,
  );

  $headings = array(
    'fundraiser' => '<h2>Fundraising enabled types</h2>',
    'webform_user' => '<h2>Other form types</h2>',
  );

  $content_types = springboard_p2p_fields_get_sorted_content_types();
  foreach (array('fundraiser', 'webform_user') as $form_type) {
    $form['form_types'][$form_type . '_items'] = array(
      '#tree' => TRUE,
    );

    if (count($content_types[$form_type])) {
      $defaults = variable_get('springboard_p2p_' . $form_type . '_items', array());
      $form['form_types'][$form_type . '_items']['#prefix'] = $headings[$form_type];

      foreach ($content_types[$form_type] as $key => $name) {
        $node_options = springboard_p2p_fields_get_node_options_by_types($key);
        $formatted_node_options = springboard_p2p_fields_format_node_options($node_options);
        $form['form_types'][$form_type . '_items'][$key] = array(
          'enabled' => array(
            '#type' => 'checkbox',
            '#title' => $name,
            '#default_value' => isset($defaults[$key]['enabled']) ? $defaults[$key]['enabled'] : 0,
          ),
          'nid' => array(
            '#type' => 'select',
            '#options' => $formatted_node_options,
            '#default_value' => isset($defaults[$key]['nid']) ? $defaults[$key]['nid'] : '',
            '#empty_value' => '',
          ),
        );
      }
    }
  }

  $form['registration_fields_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => 'Registration fields',
    '#weight' => -30,
  );

  $form['registration_fields_fieldset']['registration_fields'] = array(
    '#tree' => TRUE,
  );

  $fields = entity_get_all_property_info('user', 'user');

  $defaults = variable_get('springboard_p2p_registration_fields', array());
  $sorted_keys = springboard_p2p_sort_keys_by_weight($defaults);
  $sorted_fields = array();
  foreach ($sorted_keys as $key) {
    $sorted_fields[$key] = $fields[$key];
    unset($fields[$key]);
  }
  // Add in any leftover fields to the end.
  $sorted_fields += $fields;

  foreach ($sorted_fields as $key => $field) {
    if (springboard_p2p_is_blacklisted_registration_field($key)) {
      continue;
    }

    if (springboard_p2p_is_locked_registration_field($key)) {
      $enabled_default_value = 1;
      $enabled_disabled = TRUE;
      $required_default_value = 1;
      $required_disabled = TRUE;
    }
    else {
      $enabled_default_value = isset($defaults[$key]['enabled']) ? $defaults[$key]['enabled'] : 0;
      $enabled_disabled = FALSE;
      $required_default_value = isset($defaults[$key]['required']) ? $defaults[$key]['required'] : 0;
      $required_disabled = FALSE;
    }

    $weight_default_value = isset($defaults[$key]['weight']) ? $defaults[$key]['weight'] : 0;

    $form['registration_fields_fieldset']['registration_fields'][$key] = array(
      'name' => array(
        '#markup' => check_plain($field['label']),
      ),
      'enabled' => array(
        '#type' => 'checkbox',
        '#default_value' => $enabled_default_value,
        '#disabled' => $enabled_disabled,
      ),
      'required' => array(
        '#type' => 'checkbox',
        '#default_value' => $required_default_value,
        '#disabled' => $required_disabled,
      ),
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#delta' => 20,
        '#title_display' => 'invisible',
        '#default_value' => $weight_default_value,
      ),
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 250,
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Changes'),
  );

  return $form;
}

/**
 * Validation handler for the p2p global admin settings.
 */
function springboard_p2p_admin_settings_validate($form, &$form_state) {
  foreach (array('fundraiser_items', 'webform_user_items') as $type) {
    if (isset($form_state['values'][$type])) {
      foreach ($form_state['values'][$type] as $content_type => $item) {
        if ($item['enabled'] && $item['nid'] == '') {
          $name = $form['form_types'][$type][$content_type]['enabled']['#title'];
          $error = t('A default form must be selected for the enabled content type %content_type.', array('%content_type' => $name));
          form_set_error($type . '][' . $content_type . '][nid', $error);
        }
      }
    }
  }
}

/**
 * Extra submit handler for p2p admin settings.
 * This one handles the 'bulk generate' field.
 */
function springboard_p2p_reset_url_aliases($form, &$form_state) {
  springboard_p2p_set_pathauto_default_aliases(NULL, TRUE);
}

/**
 * Submit handler for the p2p global admin settings.
 */
function springboard_p2p_admin_settings_submit($form, &$form_state) {
  $keys = array(
    'fundraiser_items',
    'webform_user_items',
    'registration_fields',
    'login_message_area',
    'help_message_area',
    'existing_user_message_area',
  );

  foreach ($keys as $key) {
    $value = isset($form_state['values'][$key]) ? $form_state['values'][$key] : array();
    variable_set('springboard_p2p_' . $key, $value);
  }

  drupal_set_message('Settings saved.');
}

/**
 * Theme function for the p2p global admin settings.
 */
function theme_springboard_p2p_admin_settings($variables) {
  $form = $variables['form'];

  $output = '';
  $output .= drupal_render($form['basic']);

  // The content types tables.
  $header = array(t('Enabled'), t('Default form'));
  foreach (array('fundraiser_items', 'webform_user_items') as $type) {
    $rows = springboard_p2p_fields_form_types_table_rows($form, $type);
    if (count($rows)) {
      $form['form_types'][$type]['#markup'] = theme('table', array(
          'header' => $header,
          'rows' => $rows,
        ));
    }
  }

  // The registration fields table and tabledrag.
  $weight_class = 'registration-field-weight';
  $table_id = 'registration-fields-table';
  $rows = springboard_p2p_registration_fields_table_rows($form, $weight_class);
  $header = array(
    t('Name'),
    t('Show on registration form'),
    t('Required'),
    t('Weight'),
  );

  $form['registration_fields_fieldset']['registration_fields']['#markup'] = theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => $table_id),
    ));

  drupal_add_tabledrag($table_id, 'order', 'sibling', $weight_class);

  // Everything else.
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Generates a rows array for the registration fields table.
 *
 * @param array $form
 *   The admin settings form api array.
 * @param string $weight_class
 *   The class to add to the weight column for tabledrag.
 *
 * @return array
 *   An array of rows for the registration fields table.
 */
function springboard_p2p_registration_fields_table_rows(&$form, $weight_class) {
  $rows = array();
  foreach (element_children($form['registration_fields_fieldset']['registration_fields']) as $id) {
    $form['registration_fields_fieldset']['registration_fields'][$id]['weight']['#attributes']['class'] = array($weight_class);
    $rows[] = array(
      'data' => array(
        drupal_render($form['registration_fields_fieldset']['registration_fields'][$id]['name']),
        drupal_render($form['registration_fields_fieldset']['registration_fields'][$id]['enabled']),
        drupal_render($form['registration_fields_fieldset']['registration_fields'][$id]['required']),
        drupal_render($form['registration_fields_fieldset']['registration_fields'][$id]['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  return $rows;
}

/**
 * Determine if a registration field should be not be shown on the settings.
 *
 * @param string $key
 *   The key of the registration field.
 *
 * @return bool
 *   TRUE if this field should not be displayed.
 */
function springboard_p2p_is_blacklisted_registration_field($key) {
  return in_array($key, array(
      'uid',
      'name',
      'url',
      'edit_url',
      'last_access',
      'last_login',
      'created',
      'roles',
      'status',
      'theme',
      'sbp_cid',
      'sbp_ms',
      'sbp_referrer',
      'sbp_initial_referrer',
      'sbp_search_engine',
      'sbp_search_string',
      'sbp_user_agent',
      'sbp_salesforce_account_id',
      'sbp_salesforce_contact_id',
  ));
}

/**
 * Determines if the user should not be allowed to disable a registration field.
 *
 * @param string $key
 *   The registration field key.
 *
 * @return bool
 *   TRUE if the field should not be disabled.
 */
function springboard_p2p_is_locked_registration_field($key) {
  return in_array($key, array(
      'sbp_first_name',
      'sbp_last_name',
      'mail',
  ));
}

/**
 * Sorts an array by weight.
 *
 * @param array $items
 *   Associative array of items, nested with a weight key.
 *
 * @return array
 *   The item keys ordered by weight.
 */
function springboard_p2p_sort_keys_by_weight($items) {
  $sort = array();
  foreach (array_keys($items) as $key) {
    $sort[$key] = $items[$key]['weight'];
  }
  asort($sort);

  return array_keys($sort);
}
