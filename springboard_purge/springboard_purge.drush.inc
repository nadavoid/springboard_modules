<?php
/**
 * @file
 * Defines drush command for purging old data.
 */

define('SPRINGBOARD_PURGE_DEFAULT_DAYS_OLD', 90);
define('SPRINGBOARD_PURGE_DEFAULT_BATCH_SIZE', 250);

/**
 * Implements hook_drush_command().
 */
function springboard_purge_drush_command() {

  $items  = array();
  $items['springboard-purge'] = array(
    'callback'    => 'springboard_purge',
    'description' => dt('Delete old records to reduce database size, oldest first.'),
    'aliases' => array('sbp'),
    'arguments' => array(
      'type' => 'The type of record to delete. (users, webforms, or orders)',
    ),
    'options'   => array(
      'days-old' => 'The number of days old the record should be to be considered for deletion. Defaults to ' . SPRINGBOARD_PURGE_DEFAULT_DAYS_OLD,
      'excluded-statuses' => 'Additional commerce_order statuses protect when purging orders',
      'batch-size' => 'The number of records to delete. Defaults to ' . SPRINGBOARD_PURGE_DEFAULT_BATCH_SIZE,
      'protected-roles' => 'Additional roles to be protected when purging users.',
      'dry-run' => 'Display information about records that would be deleted, but don\'t actually perform any delete actions',
      'silent' => 'Disable watchdog logging. Defaults to FALSE',
    ),
    'examples' => array(
      'drush sbp user' => 'Deletes ' . SPRINGBOARD_PURGE_DEFAULT_BATCH_SIZE . ' users that have not logged in for at least ' . SPRINGBOARD_PURGE_DEFAULT_DAYS_OLD . ' days.',
      'drush sbp webforms --days-old=100 --batch-size=500' => 'Deletes 500 webform submissions at least 100 days old.',
      'drush sbp users --protected-roles="Red shirts, Brown shirts"' => 'Deletes users while ommitting specific roles (in addition to the defaults.',
    ),
  );
  return $items;
}

/**
 * Drush command callback.
 */
function springboard_purge($type = NULL) {

  $options = array(
    'users' => dt('users'),
    'webforms' => dt('webforms'),
    'orders' => dt('orders'),
    'sustainers' => dt('sustainers'),
  );
  // Check for existence of argument.
  if (!$type) {
    $type = drush_choice($options, dt('What kind of records do you want to purge?'));
  }
  // Check for correct argument.
  $correct_args = array_keys($options);
  if (!in_array($type, $correct_args)) {
    return drush_set_error(dt('"@type" is not a valid option. Please try again.', array('@type' => $type)));
  }
  // Get options.
  $days_old = drush_get_option('days-old', SPRINGBOARD_PURGE_DEFAULT_DAYS_OLD);
  $batch_size = drush_get_option('batch-size', SPRINGBOARD_PURGE_DEFAULT_BATCH_SIZE);
  $protected_roles = array_filter(array_map('trim', explode(",", drush_get_option('protected-roles'))));
  $dry_run = drush_get_option('dry-run', FALSE);
  $excluded_statuses = array_filter(array_map('trim', explode(",", drush_get_option('excluded-statuses'))));
  $silent = drush_get_option('silent', FALSE);
  $time_start = microtime(TRUE);

  if (!$dry_run && !drush_confirm(dt('Do you really want to delete these records? This can not be undone.'))) {
    drush_die('Aborting.');
  }

  switch ($type) {
    case 'users':
        if (!$silent) {
        // Count rows before deletion.
        $count_users_before = springboard_purge_count_rows('users');
        }
        // Perform the delete.
        springboard_purge_delete_users($days_old, $batch_size, $protected_roles, $dry_run);
        if ($silent) {
          return;
        }
        // Count rows after deletion and log the results.
        $count_users_after = springboard_purge_count_rows('users');
        $log['@count_users_after'] = number_format($count_users_after);
        $log['@count_users_deleted'] = number_format($count_users_before - $count_users_after);
        $log['@time_elapsed'] = round(microtime(TRUE) - $time_start, 2);
        watchdog('springboard_purge', '@count_users_deleted rows deleted from the user table in @time_elapsed seconds. @count_users_after rows remain.', $log);
      break;

    case 'webforms':
        if (!$silent) {
        // Count rows before deletion.
        $count_webform_submissions_before = springboard_purge_count_rows('webform_submissions');
        }
        // Perform the delete.
        springboard_purge_delete_webform_submissions($days_old, $batch_size, $excluded_statuses, $dry_run);
        if ($silent) {
          return;
        }
        // Report on webforms_confirmations_submissions_purge cron setting.
        springboard_purge_check_webform_confirmations_submissions_purge_enabled();
        // Count rows after deletion and log the results.
        $count_webform_submissions_after = springboard_purge_count_rows('webform_submissions');
        $log['@count_webform_submissions_after'] = number_format($count_webform_submissions_after);
        $log['@count_webform_submissions_deleted'] = $count_webform_submissions_before - $count_webform_submissions_after;
        $log['@time_elapsed'] = round(microtime(TRUE) - $time_start, 2);
        watchdog('springboard_purge', '@count_webform_submissions_deleted rows deleted from the webform_submissions table in @time_elapsed seconds. @count_webform_submissions_after rows remain.', $log);
      break;

    case 'orders':
        if (!$silent) {
        // Count rows before deletion.
        $count_commerce_order_before = springboard_purge_count_rows('commerce_order');
        $count_fundraiser_donation_before = springboard_purge_count_rows('fundraiser_donation');
        $count_fundraiser_sustainers_before = springboard_purge_count_rows('fundraiser_sustainers');
        }
        // Perform the delete.
        springboard_purge_delete_commerce_orders($days_old, $batch_size, $excluded_statuses, $dry_run);
        if ($silent) {
          return;
        }
        // Count rows after deletion and log the results.
        $count_commerce_order_after = springboard_purge_count_rows('commerce_order');
        $count_fundraiser_donation_after = springboard_purge_count_rows('fundraiser_donation');
        $count_fundraiser_sustainers_after = springboard_purge_count_rows('fundraiser_sustainers');
        $log['@count_commerce_order_after'] = number_format($count_commerce_order_after);
        $log['@count_fundraiser_donation_after'] = number_format($count_fundraiser_donation_after);
        $log['@count_fundraiser_sustainers_after'] = number_format($count_fundraiser_sustainers_after);
        $log['@count_commerce_order_deleted'] = $count_commerce_order_before - $count_commerce_order_after;
        $log['@count_fundraiser_donation_deleted'] = $count_fundraiser_donation_before - $count_fundraiser_donation_after;
        $log['@count_fundraiser_sustainers_deleted'] = $count_fundraiser_sustainers_before - $count_fundraiser_sustainers_after;
        $log['@time_elapsed'] = round(microtime(TRUE) - $time_start, 2);
        watchdog('springboard_purge', '@count_commerce_order_deleted rows deleted from the commerce_order table (@count_commerce_order_after remain), @count_fundraiser_donation_deleted rows deleted from the fundraiser_donation table (@count_fundraiser_donation_after remain), @count_fundraiser_sustainers_deleted rows deleted from the fundraiser_sustainers table (@count_fundraiser_sustainers_after remain) in @time_elapsed seconds.', $log);
      break;

    case 'sustainers':
        if (!$silent) {
        // Count rows before deletion.
        $count_commerce_order_before = springboard_purge_count_rows('commerce_order');
        $count_fundraiser_donation_before = springboard_purge_count_rows('fundraiser_donation');
        $count_fundraiser_sustainers_before = springboard_purge_count_rows('fundraiser_sustainers');
        }
        // Perform the delete.
        springboard_purge_delete_commerce_orders($days_old, $batch_size, $excluded_statuses, $dry_run, 'sustainers');
        if ($silent) {
          return;
        }
        // Count rows after deletion and log the results.
        $count_commerce_order_after = springboard_purge_count_rows('commerce_order');
        $count_fundraiser_donation_after = springboard_purge_count_rows('fundraiser_donation');
        $count_fundraiser_sustainers_after = springboard_purge_count_rows('fundraiser_sustainers');
        $log['@count_commerce_order_after'] = number_format($count_commerce_order_after);
        $log['@count_fundraiser_donation_after'] = number_format($count_fundraiser_donation_after);
        $log['@count_fundraiser_sustainers_after'] = number_format($count_fundraiser_sustainers_after);
        $log['@count_commerce_order_deleted'] = $count_commerce_order_before - $count_commerce_order_after;
        $log['@count_fundraiser_donation_deleted'] = $count_fundraiser_donation_before - $count_fundraiser_donation_after;
        $log['@count_fundraiser_sustainers_deleted'] = $count_fundraiser_sustainers_before - $count_fundraiser_sustainers_after;
        $log['@time_elapsed'] = round(microtime(TRUE) - $time_start, 2);
        watchdog('springboard_purge', '@count_commerce_order_deleted rows deleted from the commerce_order table (@count_commerce_order_after remain), @count_fundraiser_donation_deleted rows deleted from the fundraiser_donation table (@count_fundraiser_donation_after remain), @count_fundraiser_sustainers_deleted rows deleted from the fundraiser_sustainers table (@count_fundraiser_sustainers_after remain) in @time_elapsed seconds.', $log);
      break;

    default:
  }
}
