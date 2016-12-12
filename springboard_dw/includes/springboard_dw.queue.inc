<?php

/**
 * @file
 * Queue functions for Springboard data warehouse.
 */

/**
 * Queue processing hook.
 */
function springboard_dw_process_queue() {

  $callback = 'springboard_dw_export_item';

  $end = time() + 60;

  // Get an instance of the Drupal data warehouse queue.
  $queue = DrupalQueue::get('springboard_dw_export');

  // Create an array for placeholder to track any failed queue items.
  $failed_queue_items = array();

  // Lock and claim an item for queue processing.
  while (time() < $end && ($item = $queue->claimItem())) {

    // If the api responds back correctly, remove the queue item.
    if (call_user_func($callback, $item->data)) {
      try {
        $queue->deleteItem($item);
      } catch (Exception $e) {
        // In case of exception log it and leave the item in the queue
        // to be processed again later.
        watchdog_exception('cron', $e);

        // Add this failed item to the failed queue items array.
        $failed_queue_items[] = $item;
      }
    }
    else {

      // Generate the error logging string.
      $item_id = $item->item_id;
      $error_log_string = "<pre>Bad API call, releasing queue item
      $item_id.\n" . print_r($item, TRUE) . "</pre>";

      // Log this to Drush for debugging.
      drush_log($error_log_string, 'warning');

      // If watchdog logging is enabled for item processing and API request
      // failures log this queue process failure here.
      springboard_dw_watchdog_log($error_log_string);

      // Add this failed item to the failed queue items array.
      $failed_queue_items[] = $item;
    }
  }

  /*
   * Loop through any failed API call items and release them back onto the queue
   * This has to be done because releasing directly within the processing
   * loop causes an infinite loop as the failed
   * item is added back immediately to the top of the queue instead of being
   * postponed till the next queue batch
   * (Documented https://www.drupal.org/node/1524550
   * and https://www.drupal.org/node/1832818)
   */
  foreach ($failed_queue_items as $queue_item) :
    $queue->releaseItem($queue_item);
  endforeach;
}

/**
 * Implements hook_cron_queue_info().
 */
function springboard_dw_cron_queue_info() {

  $queues['springboard_dw_export'] = array(
    'worker callback' => 'springboard_dw_export_item',
    'time'            => 60,
    'skip on cron'    => TRUE,
  );

  return $queues;
}

/**
 * Query the data warehouse table to check if an item has been recorded already.
 *
 * @param string $type
 *   The type of warehouse object object to check against in the DB query.
 * @param int    $drupal_id
 *   The unique identifier for the item (typically a Drupal ID unless it is
 *   a message item in which case it is a MongoId string).
 */
function springboard_dw_item_is_warehoused($type, $drupal_id) {

  // Query items from the data warehouse table that match the item id and type.
  $query = db_query("select * from {springboard_data_warehouse} where drupal_id = :drupal_id and type = :type", array(':type'      => $type,
                                                                                                                      ':drupal_id' => $drupal_id
  ));

  // If an item is found, we do not need to warehouse this item.
  if ($query->rowCount() > 0) {
    return TRUE;
  }

  // If this item is not warehoused, return false.
  return FALSE;
}

/**
 * Mark an item in Drupal as being warehoused.
 *
 * @param string $type
 *   The type or identifier for what kind of item we are tracking.
 * @param mixed  $identifier
 *   The unique identifier for the item (typically a Drupal ID
 *   unless it is a message item in which case it is a MongoId string).
 *
 * @throws \Exception
 */
function springboard_dw_track_item_warehouse_queued($type, $identifier) {

  // Insert the user account into the Drupal data warehouse table.
  db_insert('springboard_data_warehouse')->fields(
    array(
      'type'      => $type,
      'drupal_id' => $identifier,
      'created'   => time(),
    )
  )->execute();
}
