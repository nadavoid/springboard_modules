<?php

/**
 * @file
 * Provide a Multivariate Testing win threshold: Minimum message action messages generated.
 */

// Plugin details
$plugin = array(
  'title' => t('Minimum Messages Generated'),
  'description' => t('All message actions must generated at least this many messages before the win conditions are checked.'),
  'config form' => 'sba_message_action_ab_minimum_messages_generated_config',
  'config form validation' => 'sba_message_action_ab_minimum_messages_generated_config_validate',
  'config summary' => 'sba_message_action_ab_minimum_messages_generated_config_summary',
  'check criteria' => 'sba_message_action_ab_minimum_messages_generated_check_threshold',
);

/**
 * Return the config options form for this win threshold
 *
 * @param $current_config
 *   Array containing current config options (if set). Default values for form
 *   elements should come from here.
 */
function sba_message_action_ab_minimum_messages_generated_config($current_config) {
  $form = array();
  $form['messages_generated'] = array(
    '#type' => 'textfield',
    '#title' => t('Messages Generated'),
    '#description' => t('The minimum number of messages generated.'),
    '#size' => 10,
    '#maxlength' => 255,
    '#default_value' => isset($current_config['messages_generated']) ? $current_config['messages_generated'] : '',
  );
  return $form;
}

/**
 * Validate the config for this plugin
 */
function sba_message_action_ab_minimum_messages_generated_config_validate($config, $field_name_prefix) {
  if (!ctype_digit($config['messages_generated']) || !($config['messages_generated'] > 0)) {
    form_set_error($field_name_prefix . 'messages_generated', t('Please enter an integer for the Minimum Messages Generated win threshold.'));
  }
}


/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate test node
 */
function sba_message_action_ab_minimum_messages_generated_config_summary($config) {
  return $config['messages_generated'];
}


/**
 * Check if the given Multivariate test node has a winner
 *
 * @param $config
 *   Array of config options for this plugin for the multivariate test being
 *   checked for a win. In this case, the array will contain 'hits'
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate test node
 */
function sba_message_action_ab_minimum_messages_generated_check_threshold($config, $webform_details, $test_node) {
  $messages = isset($webform_details['messages_generated']) ? $webform_details['messages_generated'] : 0;
  return ($messages >= $config['messages_generated']);
}
