<?php

/**
 * @file
 * Drush commands for the Springboard data warehouse module.
 */

/**
 * Implements hook_drush_help().
 */
function springboard_dw_drush_help($command) {

  switch ($command) {
    case 'drush:springboard-dw-export-queue-process':
      return dt('Processes the Springboard data warehouse export queue.');
  }
}

/**
 * Implements hook_drush_command().
 */
function springboard_dw_drush_command() {

  $items = array();
  $items['springboard-dw-export-queue-process'] = array(
    'description' => dt('Processes the Springboard data warehouse export queue.'),
    'examples' => array(
      'Standard example' => 'springboard-dw-export-queue-process',
    ),
    'aliases' => array('sb-dw-export'),
  );

  return $items;
}

/**
 * Callback function for drush springboard-dw-export-queue-process.
 *
 * Processes the Springboard data warehouse export queue.
 */
function drush_springboard_dw_export_queue_process() {

  // If the data warehouse config is not set, don't try to process the queue.
  if (!variable_get('springboard_dw_api_endpoint', FALSE) || !variable_get('springboard_dw_api_endpoint', FALSE)) {
    drush_print('Could not process the data warehouse queue because config has not been set.');
    springboard_dw_watchdog_log('Could not process the data warehouse queue because config has not been set.');
  }
  else {
    // Provide CLI feedback.
    drush_print('Processing the Springboard data warehouse export queue');

    // Process the items in the queue.
    springboard_dw_process_queue();

    // Log to the command line with an OK status.
    drush_log('Running springboard-dw-export-queue-process', 'ok');
    springboard_dw_watchdog_log('Running springboard-dw-export-queue-process');
  }
}
