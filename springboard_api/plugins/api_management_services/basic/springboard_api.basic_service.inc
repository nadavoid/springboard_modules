<?php

/**
 * @file
 * Test management service plugin.
 * Provides simplistic authentication, tracking, and throttling for use when
 * debugging.
 */

$plugin = array(
  'name' => 'basic',
  'title' => t('Basic API Management Service'),
  'authentication_callback' => 'springboard_api_basic_service_authenticate',
  'tracking_callback' => 'springboard_api_basic_service_track',
  'throttling_callback' => 'springboard_api_basic_service_throttle',
  'admin_settings' => 'springboard_api_basic_service_settings',
  'menu_items' => 'springboard_api_basic_service_menu_items',
);

/**
 * Authentication callback.
 */
function springboard_api_basic_service_authenticate($op, $data) {

  $api_key = FALSE;
  switch($op) {
    case 'index':
    case 'retrieve':
    case 'submit':
      $api_key = !empty($data[1]) ? $data[1] : FALSE;
      break;
  }
  if (!$api_key) {
    return FALSE;
  }
  $api_keys = variable_get('springboard_api_basic_keys', array());
  if (in_array($api_key, array_keys($api_keys)) && $api_keys[$api_key]['enabled']) {
    return TRUE;
  }
  springboard_api_log_service_usage($op, FALSE, NULL, $api_key);
  return services_error(t('Missing or invalid API key'), 406);
}

/**
 * API usage tracking callback.
 */
function springboard_api_basic_service_track($endpoint, $data = '') {
  springboard_api_log_service_usage($endpoint, $data);
}

/**
 * API usage throttling callback.
 */
function springboard_api_basic_service_throttle() {
  if (variable_get('springboard_api_basic_service_enable_throttling', 0)) {
    // Throttling enabled, check conditions.
    $limit = variable_get('springboard_api_basic_service_throttle_limit', 0);
    if ($limit > 0) {
      $count = _springboard_api_tracker_count();
      if ($count > $limit) {
        return services_error("API request limits exceeded.", 403);
      }

    }
    return TRUE;
  }
}

/**
 * Admin settings form for plugin.
 */
function springboard_api_basic_service_settings(&$form, $form_state) {
  $form['api_keys'] = array(
    '#type' => 'fieldset',
    '#title' => t('App Keys'),
    '#description' => t('API keys control access to the API and allow requests to be associated with an individual application. API keys can be enabled or disabled via the checkbox to the left of the app name.'),
    '#states' => array(
      'visible' => array(
        ':input[name="springboard_api_management_service"]' => array('value' => 'basic'),
      ),
    ),
  );

  $api_keys = variable_get('springboard_api_basic_keys', array());
  $rows = array();
  $defaults = array();
  foreach ($api_keys as $key => $value) {
    $links = _springboard_api_basic_service_links($key);
    $defaults[$key] = $value['enabled'] ? $key : 0;
    $rows[$key] = array(
      $value['name'],
      $key,
      $links,
    );
  }
  if (count($rows)) {
    $header = array(
      t('App Name'),
      t('API key'),
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
    $form['api_keys']['api_key_list'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $rows,
      '#default_value' => $defaults,
    );
  }
  $form['api_keys']['add_basic_app_name'] = array(
    '#type' => 'textfield',
    '#title' => t('App name'),
  );
  $form['api_keys']['add_submit'] = array(
    '#type' => 'button',
    '#value' => t('Add'),
    '#executes_submit_callback' => TRUE,
  );
  $form['#submit'][] = '_springboard_api_basic_service_submit';
}

/**
 * Get endpoint usage in the last minute.
 */
function _springboard_api_tracker_count() {
  $end = time();
  $start = $end - 60;
  $result = db_query('SELECT COUNT(time) FROM {springboard_api_tracker} WHERE time >= :start AND time <= :end', array(':start' => $start, ':end' => $end));
  return $result->fetchField();
}

function _springboard_api_basic_service_links($id) {
  $links = l(t('Delete'), 'admin/config/services/springboard_api/' . $id .'/delete');
  return $links;
}

function _springboard_api_basic_service_submit($form, $form_state) {
  $api_keys = variable_get('springboard_api_basic_keys', array());
  $enabled = $form_state['values']['api_key_list'];
  foreach ($api_keys as $key => $settings) {
    /*
    if ($key == '0' || !$key) {
      unset($api_keys[$key]);
      continue;
    }*/
    if (in_array($key, array_keys($enabled))) {
      $api_keys[$key]['enabled'] = $enabled[$key] ? TRUE : FALSE;
    }
  }
  variable_set('springboard_api_basic_keys', $api_keys);
  if (!empty($form_state['values']['add_basic_app_name'])) {
    $app_name = $form_state['values']['add_basic_app_name'];
    $keygen = md5($app_name . time());
    // Indexed by api key so a quick array_keys() nets a full list of available
    // keys for access checking without iterating the array.
    $api_keys[$keygen] = array(
      'name' => $app_name,
      'enabled' => 0,
    );
    variable_set('springboard_api_basic_keys', $api_keys);
  }
}
