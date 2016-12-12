<?php

/**
 * @file
 * Provide a Multivariate Testing win condition: Minimum percentage of donations
 * that are from sustainers
 */

// Plugin details
$plugin = array(
  'title' => t('Percentage of Donations That Are Sustainers'),
  'description' => t('Of the donations made on the form, at least this percentage must be sustainers in order to win.'),
  'config form' => 'fundraiser_ab_sustainers_percentage_config',
  'config form validation' => 'fundraiser_ab_sustainers_percentage_config_validate',
  'config summary' => 'fundraiser_ab_sustainers_percentage_config_summary',
  'check criteria' => 'fundraiser_ab_sustainers_percentage_check_win',
  'current value' => 'fundraiser_ab_sustainers_percentage_current_value',
  'prep for save' => 'fundraiser_ab_sustainers_percentage_prep_save',
);

/**
 * Return the config options form for this win condition
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function fundraiser_ab_sustainers_percentage_config($current_config) {
  $form = array();
  $form['percentage'] = array(
    '#type' => 'textfield',
    '#title' => t('Percentage of Donations'),
    '#description' => t('The minimum percentage of donations that must be from sustainers.'),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['percentage']) ? ($current_config['percentage'] * 100) . '%' : '',
  );
  return $form;
}

/**
 * Validate the config for this plugin
 */
function fundraiser_ab_sustainers_percentage_config_validate($config, $field_name_prefix) {
  $value = $config['percentage'];

  // If it's between 0 and 1, it's a decimal and OK
  if ($value > 0 && $value <= 1) {
    // Fine
  }
  // See if the string ends with a percentage
  elseif (substr(trim($value), -1) == '%') {
    // Strip the percent sign
    $number = rtrim(trim($value), '%');
    if (!($number > 0 && $number <= 100)) {
      form_set_error($field_name_prefix . 'percentage', t('Please enter a valid percentage for Percentage of Donations win condition. You can either enter it as a percentage (e.g., 45%) or a decimal value (e.g., 0.45).'));
    }
  }
  // Not a number
  else {
    form_set_error($field_name_prefix . 'percentage', t('Please enter a valid percentage for Percentage of Donations win condition. You can either enter it as a percentage (e.g., 45%) or a decimal value (e.g., 0.45).'));
  }
}


/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate Testing test node
 */
function fundraiser_ab_sustainers_percentage_config_summary($config) {
  if ($config['percentage'] > 0) {
    return ($config['percentage'] * 100) . '%';
  }
}


/**
 * Prep the given configuration for saving to the DB.
 * For conversion percentage, save the percentage as a decimal between 0 and 1
 */
function fundraiser_ab_sustainers_percentage_prep_save(&$config) {
  if (substr(trim($config['percentage']), -1) == '%') {
    // Strip the percent sign
    $number = rtrim(trim($config['percentage']), '%');
    // Convert to a decimal
    $config['percentage'] = $number / 100;
  }
}

/**
 * Returns the current value this win condition is measuring for a given test.
 *
 * @param $webform_nid
 *   Node ID of the webform currently being test.
 */
function fundraiser_ab_sustainers_percentage_current_value($webform_nid) {
  $overall_donation_count = db_query("SELECT nid FROM {fundraiser_donation} d WHERE d.nid = :webform_nid",
    array(':webform_nid' => $webform_nid))
    ->rowCount();
  $sustainer_donation_count = db_query("SELECT nid FROM {fundraiser_donation} d, {fundraiser_sustainers} s " .
    "WHERE d.did = s.master_did AND d.nid = :webform_nid",
    array(':webform_nid' => $webform_nid))
    ->rowCount();
  if (!$overall_donation_count) {
    return 0;
  }
  return round($sustainer_donation_count / $overall_donation_count, 2);
}


/**
 * Check if the given Multivariate Testing Test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the Multivariate Testing test being
 *   checked for a win. In this case, the array will contain 'percentage'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate Testing Test node
 */
function fundraiser_ab_sustainers_percentage_check_win($config, $webform_details, $test_node) {
  if (isset($webform_details['fundraiser']) && is_array($webform_details['fundraiser']) && ($webform_details['fundraiser']['sustainer_count'] > 0)) {
    $percentage = $webform_details['fundraiser']['sustainer_count'] / $webform_details['fundraiser']['donation_count'];
    return $percentage >= $config['percentage'];
  }
  return FALSE;
}
