<?php

/**
 * @file
 * Admin settings form and associated functions.
 */

/**
 * Admin settings form.
 */
function sb_social_admin() {

  $form['#attached']['js'][] = drupal_get_path('module', 'sb_social') . '/scripts/sb-social-admin.js';

  $form['springboard_social_debug_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable debug mode'),
    '#default_value' => variable_get('springboard_social_debug_mode', 0),
    '#description' => t('Disable the short URL conversion')
  );

  // Begin settings for users to customize block title and description.
  $form['block_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default Block Title and Description')
  );
  $form['block_info']['springboard_social_default_block_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Block Title'),
    '#default_value' => variable_get('springboard_social_default_block_title', ''),
  );

  $form['block_info']['springboard_social_default_block_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Default Block Description'),
    '#default_value' => variable_get('springboard_social_default_block_description', ''),
  );

  // Enable social settings tab on a per-content-type basis.
  $form['content_types'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enable social sharing for the following content types'),
  );
  $content_types = node_type_get_types();
  foreach ($content_types as $key => $type) {
    $content_type_options[$type->type] = $type->name;
  }
  $form['content_types']['springboard_social_enabled_content_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled content types'),
    '#options' => $content_type_options,
    '#default_value' => variable_get('springboard_social_enabled_content_types', array()),
  );

  $form['services'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enable social sharing services'),
  );
  $plugins = sb_social_list_plugins();
  foreach ($plugins as $plugin) {
    $services[$plugin['name']] = $plugin['title'];
  }
  $form['services']['springboard_social_services'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled services'),
    '#options' => $services,
    '#default_value' => variable_get('springboard_social_services', array()),
  );

  // Each plugin should implement an an admin settings callback that outputs
  // the specific form fields required by the service. These fields are used to
  // configure sitewide defaults for share content.
  foreach ($plugins as $service => $plugin) {
    $func = isset($plugin['admin_settings']) ? $plugin['admin_settings'] : '';
    if (!empty($func) && function_exists($func)) {
      $func($form);
    }
  }

  $form['addthis'] = array(
    '#type' => 'fieldset',
    '#title' => t('AddThis'),
  );

  $form['addthis']['springboard_social_use_addthis'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use AddThis'),
    '#default_value' => variable_get('springboard_social_use_addthis', 0),
    '#description' => t("If checked, Springboard Social will use AddThis instead of Springboard's native  social integration.")
  );

  $form['addthis']['springboard_social_addthis_profile_id'] = array(
    '#type' => 'textfield',
    '#title' => t('AddThis Profile ID'),
    '#default_value' => variable_get('springboard_social_addthis_profile_id', ''),
    '#states' => array(
      'visible' => array(
        ':input[name="springboard_social_use_addthis"]' => array('checked' => TRUE),
      ),
      'required' => array(
        ':input[name="springboard_social_use_addthis"]' => array('checked' => TRUE),
      ),
    ),
  );

  if (module_exists('market_source')) {
    $form['services']['market_source'] = array(
      '#type' => 'fieldset',
      '#title' => t('Market Source Settings'),
      '#description' => t('Optional Market Source values to include in share links'),
    );
    $fields = market_source_field_info();
    $share_enabled_fields = _sb_social_list_share_enabled_market_source_fields();

    foreach ($fields as $key => $field) {
      if (!in_array($key, $share_enabled_fields)) {
        continue;
      }
      // Special handling for campaign ID.
      if ($key == 'cid') {
        $campaigns = market_source_get_campaigns(FALSE);
        $select_options = array();
        foreach ($campaigns as $campaign_id => $campaign) {
          $select_options[$campaign_id] = $campaign['name'] . ' (' . $campaign_id . ')';
        }

        $form['services']['market_source']['springboard_social_ms_cid'] = array(
          '#type' => 'select',
          '#title' => t('Campaign ID'),
          '#options' => $select_options,
          '#default_value' => variable_get('springboard_social_ms_cid', ''),
        );
      }
      else {
        $form['services']['market_source']['springboard_social_ms_' . $key] = array(
          '#type' => 'textfield',
          '#title' => $field['#title'],
          '#default_value' => variable_get('springboard_social_ms_' . $key, ''),
        );
      }
    }
  }
  $form['#submit'][] = 'sb_social_admin_settings_submit';
  return system_settings_form($form);
}


/**
 * Path settings admin page.
 */
function sb_social_path_admin() {
  $form['preamble'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Manage custom social share settings for non-node pages like Views and Panels.') . '</p>',
  );
  $form['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add settings!'),
    '#limit_validation_errors' => array(),
  );

  $header = array(
    t('Path'),
    t('Actions'),
  );
  $rows = array();
  $settings = sb_social_path_settings_load_all();
  foreach ($settings as $item) {
    $row = array();
    $row[] = $item['path'];
    $row[] = _sb_social_path_actions($item['sid']);
    $rows[] = $row;
  }
  $vars = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(),
    'colgroups' => array(),
    'caption' => '',
    'sticky' => FALSE,
    'empty' => '',
  );

  $form['path_list'] = array(
    '#type' => 'markup',
    '#markup' => theme_table($vars),
  );
  $form['#submit'][] = '_sb_social_add_path';
  return $form;
}

/**
 * Helper function, redirects to path add form.
 */
function _sb_social_add_path() {
  drupal_goto('admin/config/services/springboard_social/path-settings/add');
}

/**
 * Generate edit/delete links for path admin table.
 *
 * @param int $sid
 *   Settings ID
 *
 * @return string
 *   Returns links to edit & delete forms for current path settings.
 */
function _sb_social_path_actions($sid) {
  if (!$sid) {
    return FALSE;
  }
  $actions = array();

  $links[] = l(t('Edit'), 'admin/config/services/springboard_social/path-settings/edit/' . $sid);
  $links[] = l(t('Delete'), 'admin/config/services/springboard_social/path-settings/delete/' . $sid);

  return implode('&nbsp;&nbsp;', $links);
}

/**
 * Submit handler for admin settings form.
 *
 * Why are we resetting a bunch of token cache stuff here?
 */
function sb_social_admin_settings_submit($form, $form_state) {
  $plugins = sb_social_list_plugins();
  foreach ($plugins as $service => $plugin) {
    $submit_func = isset($plugin['admin_setings_submit']) ? $plugin['admin_setings_submit'] : FALSE;
    if ($submit_func && function_exists($submit_func)) {
      $submit_func($form, $form_state);
    }
  }
  cache_clear_all('*', 'cache_token', TRUE);
  drupal_static_reset('token_get_info');
  drupal_static_reset('token_get_global_token_types');
  drupal_static_reset('token_get_entity_mapping');
  drupal_static_reset('token_build_tree');
  drupal_static_reset('_token_profile_fields');
  drupal_static_reset('token_menu_link_load');
  drupal_static_reset('token_book_link_load');
  drupal_static_reset('token_node_menu_link_load');
  drupal_static_reset('_token_field_info');
}

/**
 * Path settings form.
 */
function sb_social_path_settings($form, $edit, $sid = NULL) {

  $enabled_services = sb_social_get_enabled_services();
  $settings = sb_social_settings_load($sid);

  // Differentiate add vs edit forms, prevents path validation from incorrectly
  // flagging an edit form as a dupe on hook_validate.
  $edit['is_new'] = $sid === NULL ? TRUE : FALSE;
  // Since sb_social_settings_load() will load global defaults if no settings
  // are currently available, we override the default ID for new paths. This
  // prevents the url of the path add page from getting populated in the path
  // field when creating new path settings.
  if ($edit['is_new']) {
    $settings['id'] = '';
    $form['is_new'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
  }

  $token_set = array('user', 'node');

  if (empty($enabled_services)) {
    $form['banner'] = array(
      '#prefix' => '<h2>' . t('No services enabled') . '</h2>',
      '#markup' => '<p>' . t('No social media services have been enabled. To enable Social Media services, visit the !admin_screen', array('!admin_screen' => l(t('module administration page'), 'admin/config/services/springboard_social'))) . '</p>',
    );
  }
  // Defines record type in {sb_social_settings}.
  $form['type'] = array(
    '#type' => 'value',
    '#value' => 'path',
  );

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#description' => t('Drupal internal path (Example: node/1). You may also use valid path aliases.'),
    '#default_value' => isset($settings['path']) ? $settings['path'] : '',
    '#required' => TRUE,
  );
  foreach ($enabled_services as $service) {
    $form['update_' . $service] = array(
      '#type' => 'value',
      '#value' => !empty($settings[$service]),
    );
  }

  $plugins = sb_social_list_plugins();

  // Each plugin should implement a settings callback function that provides
  // the required form elements to configure share content for this service.
  foreach ($plugins as $service => $plugin) {
    $func = isset($plugin['settings']) ? $plugin['settings'] : FALSE;
    if ($func && function_exists($func)) {
      $func($form, $enabled_services, $settings['data'][$service], $token_set);
    }
  }

  // Market Source settings.
  if (module_exists('market_source')) {
    _sb_social_settings_market_source($form, $settings);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['#validate'][] = 'sb_social_path_settings_validate';
  $form['#submit'][] = 'sb_social_path_settings_submit';
  return $form;
}

/**
 * Validation callback for the path settings form.
 */
function sb_social_path_settings_validate(&$form, $form_state) {

  if ($form_state['is_new']) {
    $path_is_alias = FALSE;
    $path = $form_state['values']['path'];
    // Does path have an alias?
    $alias = drupal_get_path_alias($path);
    if (!$alias) {
      // No alias? Check to see if path is an alias.
      $alias = drupal_lookup_path('alias', $path);
      if ($alias) {
        $path_is_alias = TRUE;
      }
    }
    // If path has an alias or is an alias, check both values for dupes.
    $id = $alias ? array($path, $alias) : $path;

    if ($sid = _sb_social_sid_by_id($id)) {
      $link = l(t('here'), 'admin/config/services/springboard_social/path-settings/edit/' . $sid);
      if (is_array($id)) {
        $message = t('!alias is a path alias for !path. Settings already exist for !path. You may edit them !link', array('!path' => $path, '!link' => $link));
      }
      elseif ($path_is_alias) {
        $message = t('!path is a path alias of !alias. Settings already exist for !alias. You may edit them !link', array('!path' => $path, '!alias' => $alias, 'link' => $link));
      }
      else {
        $message = t('Settings already exist for !path. You may edit them !link', array('!path' => $path, '!link' => $link));
      }
      form_set_error('path', $message);
    }
  }

}


/**
 * Path settings submit callback.
 *
 * Invokes social_networks plugin "process" callbacks to build data array and
 * saves data to db.
 */
function sb_social_path_settings_submit($form, &$form_state) {
  $plugins = sb_social_list_plugins();

  $data = array();
  $enabled_services = sb_social_get_enabled_services();
  $keys = array_keys($plugins);

  // TODO: replace this code block and others like it with a helper function
  // that executes the desired service callback(s).
  if (is_array($enabled_services)) {
    foreach ($enabled_services as $service) {
      if (in_array($service, $keys)) {
        $func = $plugins[$service]['process'];
        $func($data, $form_state);
      }
    }

    if (module_exists('market_source')) {
      $fields = market_source_field_info();
      foreach ($fields as $key => $field) {
        if (isset($form_state['values'][$key])) {
          $data['market_source'][$key] = $form_state['values'][$key];
        }
      }
    }
    // Check for an existing path id, if none exists save path and use ID
    // supplied.
    $id  = sb_social_get_path_id($form_state['values']['path']);
    if (!$id) {
      $id = sb_social_save_path($form_state['values']['path']);
    }
    $record = array(
      'id' => $id,
      'type' => 'path',
      'data' => $data,
    );
    sb_social_settings_save($record);
    $form_state['redirect'] = url('admin/config/services/springboard_social/path-settings', array('absolute' => TRUE));
  }
}

/**
 * Path settings delete form.
 */
function sb_social_path_settings_delete_form($form, $form_state, $sid) {
  $path = sb_social_get_id_by_sid($sid);
  $form['confirmation_text'] = array(
    '#markup' => t('Are you sure you want to unrecoverably delete the share settings for %path?', array('%path' => $path)),
  );
  $form['sid'] = array(
    '#type' => 'value',
    '#value' => $sid,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('_sb_social_path_delete_cancel'),
    '#limit_validation_errors' => array(),
  );
  $form['#submit'][] = 'sb_social_path_settings_delete_submit';
  return $form;
}

/**
 * Submit handler for path settings delete form.
 */
function sb_social_path_settings_delete_submit($form, &$form_state) {
  if (!empty($form_state['values']['sid'])) {
    sb_social_settings_delete($form_state['values']['sid']);
  }
  $form_state['redirect'] = array('admin/config/services/springboard_social/path-settings');
}

/**
 * Submit callback, drives cancel button on path settings delete form.
 */
function _sb_social_path_delete_cancel() {
  drupal_goto('admin/config/services/springboard_social/path-settings');
}

/**
 * Node level settings.
 */
function sb_social_node_settings($form, $edit, $node) {

  $enabled_services = sb_social_get_enabled_services();
  $sid = _sb_social_sid_by_id($node->nid);
  $settings = sb_social_settings_load($sid);

  // Setup for token replacement.
  $token_set = array('user', 'node');
  if (fundraiser_is_donation_type($node->type)) {
    $token_set[] = 'donation';
  }

  if (empty($enabled_services)) {
    $form['banner'] = array(
      '#prefix' => '<h2>' . t('No services enabled') . '</h2>',
      '#markup' => '<p>' . t('No social media services have been enabled. To enable Social Media services, visit the !admin_screen', array('!admin_screen' => l(t('module administration page'), 'admin/config/services/springboard_social'))) . '</p>',
    );
    return $form;
  }

  $form['id'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => 'node',
  );

  // Begin settings for users to customize block title and description.
  $form['block_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Block Title and Description'),
    '#collapsible' => TRUE
  );
  $form['block_info']['block_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Block Title'),
    '#default_value' => array_key_exists('block_title', $settings['data']) ? $settings['data']['block_title'] : variable_get('springboard_social_default_block_title', ''),
  );

  $form['block_info']['block_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Block Description'),
    '#default_value' => array_key_exists('block_description', $settings['data']) ? $settings['data']['block_description'] : variable_get('springboard_social_default_block_description', ''),
  );

  $plugins = sb_social_list_plugins();

  foreach ($enabled_services as $service) {
    if (!empty($plugins[$service]['settings'])) {
      $func = $plugins[$service]['settings'];
      $func($form, $enabled_services, $settings['data'][$service], $token_set);
    }
  }

  // Market Source settings.
  if (module_exists('market_source')) {
    _sb_social_settings_market_source($form, $settings);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#submit'][] = 'sb_social_settings_submit';
  return $form;
}

/**
 * Node settings form submit handler.
 *
 * Invokes "process" callback for each enabled plugin, actual data handling
 * happens in these callbacks.
 */
function sb_social_settings_submit($form, $form_state) {
  $plugins = sb_social_list_plugins();

  $data = array();
  $enabled_services = sb_social_get_enabled_services();
  $keys = array_keys($plugins);

  if (is_array($enabled_services)) {
    foreach ($enabled_services as $service) {
      if (in_array($service, $keys)) {
        $func = $plugins[$service]['process'];
        $func($data, $form_state);
      }
    }
    if (module_exists('market_source')) {
      $fields = market_source_field_info();
      foreach ($fields as $key => $field) {
        if (isset($form_state['values'][$key])) {
          $data['market_source'][$key] = $form_state['values'][$key];
        }
      }
    }
    $data['block_title'] = $form_state['values']['block_title'];
    $data['block_description'] = $form_state['values']['block_description'];
    // Sid is handled in the save function.
    $record = array(
      'id' => $form_state['values']['id'],
      'type' => $form_state['values']['type'],
      'data' => $data,
    );
    sb_social_settings_save($record);
    drupal_set_message(t('Settings saved.'));
  }
}

/**
 * Generate market source fields for share settings forms.
 *
 * @param array $form
 *   FAPI array.
 *
 * @param array $settings
 *   Social settings for the current context.
 */
function _sb_social_settings_market_source(&$form, $settings) {
  $form['market_source'] = array(
    '#type' => 'fieldset',
    '#title' => t('Market Source Settings'),
    '#description' => t('Optional Market Source values to include in share links'),
  );
  $fields = market_source_field_info();
  $share_enabled_fields = _sb_social_list_share_enabled_market_source_fields();

  foreach ($fields as $key => $field) {
    // Skip market source fields that haven't been share enabled.
    if (!in_array($key, $share_enabled_fields)) {
      continue;
    }
    if ($key == 'cid') {
      $campaigns = market_source_get_campaigns(FALSE);
      $select_options = array();
      foreach ($campaigns as $campaign_id => $campaign) {
        $select_options[$campaign_id] = $campaign['name'] . ' (' . $campaign_id . ')';
      }
      $form['market_source']['cid'] = array(
        '#type' => 'select',
        '#title' => t('Campaign ID'),
        '#options' => $select_options,
        '#default_value' => (!empty($settings) && !empty($settings['data']['market_source'][$key])) ? $settings['data']['market_source'][$key] : '',
      );
    }
    else {
      $form['market_source'][$key] = array(
        '#type' => 'textfield',
        '#title' => $field['#title'],
        '#default_value' => (!empty($settings) && !empty($settings['data']['market_source'][$key])) ? $settings['data']['market_source'][$key] : '',
      );
    }
  }
}
