<?php

/**
 * @file
 * Provide a Multivariate test win condition: Minimum number of sustainer
 * donations
 */

// Plugin details
$plugin = array(
  'title' => t('Number of Sustainer Donations'),
  'description' => t('A donation form must receive at least this many sustainer donations to be can be declared the winner.'),
  'config form' => 'fundraiser_ab_sustainers_count_config',
  'config form validation' => 'fundraiser_ab_sustainers_count_config_validate',
  'config summary' => 'fundraiser_ab_sustainers_count_config_summary',
  'current value' => 'fundraiser_ab_sustainers_count_current_value',
  'check criteria' => 'fundraiser_ab_sustainers_count_check_win',
);

/**
 * Return the config options form for this win condition
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function fundraiser_ab_sustainers_count_config($current_config) {
  $form = array();
  $form['sustainers'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of Sustainer Donations'),
    '#description' => t('The minimum number of sustainer donations.'),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['sustainers']) ? $current_config['sustainers'] : '',
  );
  return $form;
}

/**
 * Validate the config for this plugin
 */
function fundraiser_ab_sustainers_count_config_validate($config, $field_name_prefix) {
  if (!ctype_digit($config['sustainers']) || !($config['sustainers'] > 0)) {
    form_set_error($field_name_prefix . 'hits', t('Please enter an integer for Number of Sustainer Donations win condition.'));
  }
}


/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate test node
 */
function fundraiser_ab_sustainers_count_config_summary($config) {
  return $config['sustainers'];
}

/**
 * Returns the current value this win condition is measuring for a given test.
 *
 * @param $webform_nid
 *   Node ID of the webform currently being test.
 */
function fundraiser_ab_sustainers_count_current_value($webform_nid) {
  return db_query("SELECT nid FROM {fundraiser_donation} d, {fundraiser_sustainers} s " .
    "WHERE d.did = s.master_did AND d.nid = :webform_nid",
    array(':webform_nid' => $webform_nid))
    ->rowCount();
}

/**
 * Check if the given Multivariate test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the multivariate test being
 *   checked for a win. In this case, the array will contain 'sustainers'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate Test node
 */
function fundraiser_ab_sustainers_count_check_win($config, $webform_details, $test_node) {
  return isset($webform_details['fundraiser']) && is_array($webform_details['fundraiser']) && ($webform_details['fundraiser']['sustainer_count'] >= $config['sustainers']);
}
