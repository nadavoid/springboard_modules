<?php

/**
 * @file
 * Provide a Multivariate test win condition: Total sum of donations
 */

// Plugin details
$plugin = array(
  'title' => t('Sum of Donation Amounts'),
  'description' => t('A donation form must receive at least this much in donations to be can be declared the winner.'),
  'config form' => 'fundraiser_ab_donations_total_config',
  'config form validation' => 'fundraiser_ab_donations_total_config_validate',
  'config summary' => 'fundraiser_ab_donations_total_config_summary',
  'check criteria' => 'fundraiser_ab_donations_total_check_win',
  'current value' => 'fundraiser_ab_donations_total_current_value',
);

/**
 * Return the config options form for this win condition
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function fundraiser_ab_donations_total_config($current_config) {
  $form = array();
  $form['donation_sum'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum Sum of Donations'),
    '#description' => t('The minimum amount of donations that must be made.'),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['donation_sum']) ? $current_config['donation_sum'] : '',
  );
  return $form;
}

/**
 * Validate the config for this plugin
 */
function fundraiser_ab_donations_total_config_validate($config, $field_name_prefix) {
  if (!is_numeric($config['donation_sum']) || !($config['donation_sum'] > 0)) {
    form_set_error($field_name_prefix . 'hits', t('Please enter a positive number for Minimum Sum of Donations win condition.'));
  }
}


/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * multivariate test node
 * @todo: format this as a dollar amount
 */
function fundraiser_ab_donations_total_config_summary($config) {
  return money_format('$%.2n', $config['donation_sum']);
}

/**
 * Returns the current value this win condition is measuring for a given test.
 *
 * @param $webform_nid
 *   Node ID of the webform currently being test.
 */
function fundraiser_ab_donations_total_current_value($webform_nid) {
  $sum = db_query("SELECT SUM(amount) FROM {fundraiser_donation} WHERE nid = :webform_nid",
    array(':webform_nid' => $webform_nid))
    ->fetchField();
  if (!$sum) {
    return 0;
  }
  return $sum;
}

/**
 * Check if the given multivariate test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the multivariate test being
 *   checked for a win. In this case, the array will contain 'donation_sum'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate Testing test node
 */
function fundraiser_ab_donations_total_check_win($config, $webform_details, $test_node) {
  return isset($webform_details['fundraiser']) && is_array($webform_details['fundraiser']) && ($webform_details['fundraiser']['total_donations'] >= $config['donation_sum']);
}
