<?php

/**
 * @file
 * Provide a Multivariate Testing win threshold: Minimum actions completed. 
 */

// Plugin details
$plugin = array(
  'title' => t('Minimum Actions Completed'),
  'description' => t('All message actions must hve completed at least this many actions before the win conditions are checked.'),
  'config form' => 'sba_message_action_ab_minimum_actions_completed_config',
  'config form validation' => 'sba_message_action_ab_minimum_actions_completed_config_validate',
  'config summary' => 'sba_message_action_ab_minimum_actions_completed_config_summary',
  'check criteria' => 'sba_message_action_ab_minimum_actions_completed_check_threshold',
);

/**
 * Return the config options form for this win threshold
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function sba_message_action_ab_minimum_actions_completed_config($current_config) {
  $form = array();
  $form['actions_completed'] = array(
    '#type' => 'textfield',
    '#title' => t('Actions Completed'),
    '#description' => t('The minimum number of actions completed.'),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['actions_completed']) ? $current_config['actions_completed'] : '',
  );
  return $form;
}

/**
 * Validate the config for this plugin
 */
function sba_message_action_ab_minimum_actions_completed_config_validate($config, $field_name_prefix) {
  if (!ctype_digit($config['actions_completed']) || !($config['actions_completed'] > 0)) {
    form_set_error($field_name_prefix . 'actions_completed', t('Please enter an integer for the Minimum Actions Completed win threshold.'));
  }
}


/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate Testing node
 */
function sba_message_action_ab_minimum_actions_completed_config_summary($config) {
  return $config['actions_completed'];
}


/**
 * Check if the given Multivariate test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the Multivariate test being
 *   checked for a win. In this case, the array will contain 'hits'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate test node
 */
function sba_message_action_ab_minimum_actions_completed_check_threshold($config, $webform_details, $test_node) {
  $messages = isset($webform_details['actions_completed']) ? $webform_details['actions_completed'] : 0;
  return ($messages >= $config['actions_completed']);
}
