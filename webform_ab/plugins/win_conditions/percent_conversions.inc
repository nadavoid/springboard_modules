<?php

/**
 * @file
 * Provide a Multivariate Test win condition: Percentage of hits that result in
 * conversions
 */

// Plugin details
$plugin = array(
  'title' => t('Conversion Percentage'),
  'description' => t('A webform must reach this percentage of conversions to be declared the winner.'),
  'config form' => 'webform_ab_percent_conversions_config',
  'config form validation' => 'webform_ab_percent_conversions_config_validate',
  'config summary' => 'webform_ab_percent_conversions_config_summary',
  'check criteria' => 'webform_ab_percent_conversions_check_win',
  'prep for save' => 'webform_ab_percent_conversions_prep_save',
  'current value' => 'webform_ab_percent_conversions_current_value',
);

/**
 * Return the config options form for this win condition
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function webform_ab_percent_conversions_config($current_config) {
  $form = array();
  $form['percentage'] = array(
    '#type' => 'textfield',
    '#title' => t('Percentage'),
    '#description' => t('Percentage of hits that become conversions. Enter either as 25% or 0.25.'),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['percentage']) ? ($current_config['percentage'] * 100) . '%' : '',
  );
  return $form;
}

/**
 * Validate the config for this plugin
 */
function webform_ab_percent_conversions_config_validate($config, $field_name_prefix) {
  $value = $config['percentage'];

  // If it's between 0 and 1, it's a decimal and OK
  if ($value > 0 && $value <= 1) {
    // Fine
  }
  // See if the string ends with a percentage
  elseif (drupal_substr(trim($value), -1) == '%') {
    // Strip the percent sign
    $number = rtrim(trim($value), '%');
    if (!($number > 0 && $number <= 100)) {
      form_set_error($field_name_prefix . 'percentage', t('Please enter a valid percentage for Conversion Percentage win condition. You can either enter it as a percentage (e.g., 45%) or a decimal value (e.g., 0.45).'));
    }
  }
  // Not a number
  else {
    form_set_error($field_name_prefix . 'percentage', t('Please enter a valid percentage for Conversion Percentage win condition. You can either enter it as a percentage (e.g., 45%) or a decimal value (e.g., 0.45).'));
  }
}

/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate test node
 */
function webform_ab_percent_conversions_config_summary($config) {
  if ($config['percentage'] > 0) {
    return ($config['percentage'] * 100) . '%';
  }
}

/**
 * Prep the given configuration for saving to the DB.
 * For conversion percentage, save the percentage as a decimal between 0 and 1
 */
function webform_ab_percent_conversions_prep_save(&$config) {
  if (drupal_substr(trim($config['percentage']), -1) == '%') {
    // Strip the percent sign
    $number = rtrim(trim($config['percentage']), '%');
    // Convert to a decimal
    $config['percentage'] = $number / 100;
  }
}

/**
 * Returns the current value of this win condition.
 *
 * @param $webform_nid
 *   Note ID for the webform being tested.
 */
function webform_ab_percent_conversions_current_value($webform_nid) {
  $query_data = array(':webform_nid' => $webform_nid);
  $conversions = db_query("SELECT webform_nid FROM {webform_ab_conversion} WHERE webform_nid = :webform_nid",
    $query_data)->rowCount();
  $hits = db_query("SELECT webform_nid FROM {webform_ab_hits} WHERE webform_nid = :webform_nid",
    $query_data)->rowCount();

  $value = $hits > 0 ? $conversions / $hits : 0;
  return round($value, 2);
}

/**
 * Check if the given Multivariate test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the Multivariate test being
 *   checked for a win. In this case, the array will contain 'percentage'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate test node
 */
function webform_ab_percent_conversions_check_win($config, $webform_details, $test_node) {
  if ($webform_details['hits'] > 0) {
    $percentage = $webform_details['conversions'] / $webform_details['hits'];
    return $percentage >= $config['percentage'];
  }
  return FALSE;
}
