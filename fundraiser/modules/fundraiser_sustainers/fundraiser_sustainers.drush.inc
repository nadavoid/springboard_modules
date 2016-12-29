<?php

/**
 * @file
 * Drush commands for the fundraiser sustainers module.
 */

/**
 * Implements hook_drush_help().
 */
function fundraiser_sustainers_drush_help($command) {
  switch ($command) {
    case 'drush:fundraiser-sustainers-process':
      return dt('Processes any sustainer payments due to be charged at this point in time.');
  }
}

/**
 * Implements hook_drush_command().
 */
function fundraiser_sustainers_drush_command() {
  $items = array();
  $items['fundraiser-sustainers-process'] = array(
    'description' => dt('Processes any sustainer payments due to be charged at this point in time.'),
    'options' => array(
      'limit' => 'The maximum number of transactions to process.',
    ),
    'examples' => array(
      'Standard example' => 'fundraiser-sustainers-process',
    ),
    'aliases' => array('fsp'),
  );

  return $items;
}

/**
 * Drush command to process recurring donations.
 *
 * @param $max
 *   The maximum number of transactions to process.
 */
function drush_fundraiser_sustainers_process() {
  $limit = drush_get_option('limit', 200);

  drush_log('Checking for sustainer key match.', 'ok');
  $processor_key_match = fundraiser_sustainers_processor_key_match();

  // If the site-wide sustainer key does not match, bail out of processing.
  if (!$processor_key_match) {
    watchdog('fundraiser_sustainers_drush', 'The fundraiser sustainer key has not been configured correctly. Recurring donations will not be processed.', NULL, WATCHDOG_CRITICAL);
    return;
  }

  // Give other modules a chance to bail out of sustainer processing.
  $process_recurring = TRUE;
  drupal_alter('fundraiser_sustainers_process_recurring', $process_recurring);
  // If process_recurring is not true, bail out of processing.
  if (!$process_recurring) {
    return;
  }

  // Do one last double check before proceeding.
  if ($process_recurring && $processor_key_match) {
    drush_log(dt('All sustainer preprocessing checks complete. Starting to process !limit sustainers.', array('!limit' => $limit)), 'ok');
    
    fundraiser_sustainers_process_recurring_donations($limit);
  }
}
