<?php

/**
 * @file
 * Provide a Multivariate Testing win condition: Total number of conversions
 */

// Plugin details
$plugin = array(
  'title' => t('Total Conversions'),
  'description' => t('This many conversions must be made by a webform before it is declared the winner.'),
  'config form' => 'webform_ab_total_conversions_config',
  'config form validation' => 'webform_ab_total_conversions_config_validate',
  'config summary' => 'webform_ab_total_conversions_config_summary',
  'check criteria' => 'webform_ab_total_conversions_check_win',
  'current value' => 'webform_ab_total_conversions_current_value',
);


/**
 * Return the config options form for this win condition
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function webform_ab_total_conversions_config($current_config) {
  $form = array();
  $form['conversions'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of Conversions'),
    '#description' => t(''),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['conversions']) ? $current_config['conversions'] : '',
  );
  return $form;
}


/**
 * Validate the config for this plugin
 */
function webform_ab_total_conversions_config_validate($config, $field_name_prefix) {
  if (!ctype_digit($config['conversions']) || !($config['conversions'] > 0)) {
    form_set_error($field_name_prefix . 'conversions', t('Please enter an integer for Number of Conversions win condition.'));
  }
  return TRUE;
}


/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate test node
 */
function webform_ab_total_conversions_config_summary($config) {
  return $config['conversions'];
}

/**
 * Returns the current value of this win condition.
 *
 * @param $webform_nid
 *   Note ID for the webform being tested.
 */
function webform_ab_total_conversions_current_value($webform_nid) {
  return db_query("SELECT webform_nid FROM {webform_ab_conversion} WHERE webform_nid = :webform_nid",
    array(':webform_nid' => $webform_nid))
    ->rowCount();
}

/**
 * Check if the given Multivariate Test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the Multivariate test being
 *   checked for a win. In this case, the array will contain 'conversions'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate test node
 */
function webform_ab_total_conversions_check_win($config, $webform_details, $test_node) {
  return isset($webform_details['conversions']) ? $webform_details['conversions'] >= $config['conversions'] : FALSE;
}
