<?php

/**
 * @file
 * Admin settings screens and associated helper functions.
 */

/**
 * Admin settings form.
 */
function springboard_api_admin_settings($form, $form_state) {
  $available_services = springboard_api_list_management_services();
  $options = array_merge(array('0' => t('- select -')), $available_services);
  $form['springboard_api_management_service'] = array(
    '#type' => 'select',
    '#title' => t('API Management Service'),
    '#description' => t('Select which API management service you would like to use.'),
    '#options' => $options,
    '#default_value' => variable_get('springboard_api_management_service', 0),
  );
  // TODO: link to http://enable-cors.org/
  $form['cors'] = array(
    '#type' => 'fieldset',
    '#title' => t('CORS settings'),
    '#description' => t('Certain API client applications may need to POST data cross-domain. To accomplish this !link. If your site administrator is unable to enable CORS on the server due to access restrictions, enabling it here may work.', array('!link' => l(t('CORS must be enabled'), 'http://enable-cors.org/'))),
  );
  $form['cors']['springboard_api_enable_cors'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable CORS'),
    '#return_value' => TRUE,
    '#default_value' => variable_get('springboard_api_enable_cors', FALSE),
  );
  // Additional plugin-specific settings.
  $plugins = springboard_api_get_available_plugins();
  foreach ($plugins as $plugin) {
    if (isset($plugin['admin_settings']) && function_exists($plugin['admin_settings'])) {
      $func = $plugin['admin_settings'];
      $func($form, $form_state);
    }
  }
  return system_settings_form($form);
}


/**
 * Confirm form for api key delete.
 *
 * @param string $id
 *   The app id.
 */
function springboard_api_admin_remove_app_form($form, $form_state, $id) {
  $form = array();
  $api_keys = variable_get('springboard_api_basic_keys', array());
  $form['#app_id'] = $id;
  $title = $api_keys["{$id}"]['name'];
  $form['#submit'][] = 'springboard_api_remove_app_form_submit';
  return confirm_form(
    $form,
    t('Are you sure you want to delete %title?', array('%title' => $title)),
    'admin/config/services/springboard_api',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel'),
    'springboard_api_admin_manage_app');
}

/**
 * Remove an API key.
 */
function springboard_api_remove_app_form_submit($form, &$form_state) {
  $api_keys = variable_get('springboard_api_basic_keys', array());
  unset($api_keys[$form['#app_id']]);
  variable_set('springboard_api_basic_keys', $api_keys);
  $form_state['redirect'] = 'admin/config/services/springboard_api';
}
