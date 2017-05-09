<?php
/**
 * @file
 * Drush command and supporting functions.
 */

use GuzzleHttp\Client;

/**
 * Implements hook_drush_help().
 */
function springboard_dw_queue_legacy_drush_help($command) {
  switch ($command) {
    case 'drush:springboard-dw-queue-legacy-data':
      return dt('Add legacy data to Springboard data warehouse export queue.');
  }
}

/**
 * Implements hook_drush_command().
 */
function springboard_dw_queue_legacy_drush_command() {
  $items = array();
  $items['springboard-dw-queue-legacy-data'] = array(
    'callback' => 'springboard_dw_queue_legacy_setup_batch',
    'description' => dt('Add legacy data to Springboard data warehouse export queue.'),
    'arguments' => array(
      'type' => 'The type of record to be queued',
    ),
    'options' => array(
      'limit' => 'limit',
      'batch-size' => 'batch-size (Default 100)',
      'reset' => 'reset',
      'idlist' => 'idlist',
    ),
    'examples' => array(
      'drush sb-dw-queue contact' => 'Queues all users',
      'drush sb-dw-queue action --limit=500' => 'Queues next 500 advocacy actions.',
      'drush sb-dw-queue form --reset --limit=500' => 'Reset highwater mark to zero. Queues 500 forms.',
      'drush sb-dw-queue form --idlist=5,8,13' => 'Queues 3 forms by specific ID.',
      'drush sb-dw-queue donation --batch-size=10' => 'Queue all donations, 10 at a time. Defaults to 100. Potentially useful if you run into memory issues.',
    ),
    'aliases' => array('sb-dw-queue'),
  );

  $items['springboard-dw-queue-legacy-missing'] = array(
    'callback' => 'springboard_dw_queue_legacy_missing',
    'description' => dt('Find records missing from warehouse by type'),
    'arguments' => array(
      'type' => 'The type of record',
    ),
    'options' => array(
      'auto-queue' => 'auto-queue',
    ),
    'aliases' => array('sb-dw-missing'),
  );

  return $items;
}

/**
 * Callback function for springboard-dw-queue-legacy-missing
 */
function springboard_dw_queue_legacy_missing($type = '') {
  // Check for existence of argument.
  if (!$type) {
    $options = array(
      'contact' => dt('contact'),
      'form' => dt('form'),
      'submission' => dt('submission'),
      'donation' => dt('donation'),
      'action' => dt('action'),
      'message' => dt('message'),
      'petition_message' => dt('petition_message'),
      'payment' => dt('payment'),
      'salesforce_sync' => dt('salesforce_sync'),
    );
    $type = drush_choice($options, dt('What kind of record would you like to queue?'));
  }
  // Return only missing records by passing the $find_difference parameter as
  // TRUE.
  $records = springboard_dw_queue_legacy_get_missing_records($type);
  // Create array that only contains the IDs.
  $missing_ids = array_map(function($records) {
      return is_object($records) ? $records->id : $records['id'];
  }, $records);

  // $missing_ids = array_diff($ids, $ids_in_dw);

  drush_log(dt('Found @count records of type @type missing from the springboard_data_warehouse table', array('@count' => count($missing_ids), '@type' => $type)), 'ok');
  if (count($missing_ids) > 0) {
    drush_log(dt('They are: @list', array('@list' => implode(',', $missing_ids))), 'ok');

    if (drush_get_option('auto-queue')) {
      drush_log(dt('Attempting to queue these records.'), 'ok');
      drush_invoke_process(drush_sitealias_get_record('@self'), 'springboard-dw-queue-legacy-data', array($type), array('idlist' => implode(',', $missing_ids)));
    }
  }
}

/**
 * Callback function for drush springboard-dw-queue-legacy-data.
 *
 * Processes the Springboard data warehouse export queue.
 */
function springboard_dw_queue_legacy_setup_batch($type = '') {
  // Check for existence of argument.
  if (!$type) {
    $options = array(
      'contact' => dt('contact'),
      'form' => dt('form'),
      'submission' => dt('submission'),
      'donation' => dt('donation'),
      'action' => dt('action'),
      'message' => dt('message'),
      'petition_message' => dt('petition_message'),
      'payment' => dt('payment'),
      'salesforce_sync' => dt('salesforce_sync'),
    );
    $type = drush_choice($options, dt('What kind of record would you like to queue?'));
  }

  // Special snowflake checks for 'message' type.
  if ($type == 'message') {
    // Exit early if the API endpoint and auth token have not been set.
    if (variable_get('springboard_dw_advocacy_authentication_token') == NULL
          || variable_get('springboard_dw_advocacy_api_endpoint') == NULL
      ) {
      watchdog('data_warehouse','API endpoint and/or authentication token
      have not been set.', NULL, WATCHDOG_ERROR, l(t('Fix this on the config page'),
        'admin/config/services/springboard-dw'));
      return drush_set_error(dt('API endpoint and/or authentication token have not been set.'));
    }
    // Exit early if trying to use the --reset option on the 'message' type.
    if (drush_get_option('reset')) {
      return drush_set_error(dt('Type "message" cannot be run with the --reset option, as the highwater mark is not supported via the Advocacy service.'));
    }
    // Exit early if trying to use the --idlist option on the 'message' type.
    if (drush_get_option('idlist')) {
      return drush_set_error(dt('Type "message" cannot be run with the --idlist option, as is not supported via the Advocacy service.'));
    }
  }

  // Get options.
  $limit = drush_get_option('limit', FALSE);
  $batch_size = drush_get_option('batch-size', 100);
  if ($idlist = drush_get_option('idlist', FALSE)) {
    $idlist = explode(',', $idlist);
  }

  // Reset highwater mark to 0, if option is set.
  if (drush_get_option('reset', FALSE)) {
    springboard_dw_queue_legacy_set_highwater_mark($type, 0, $idlist);
    drush_log(dt('Resetting highwater mark to zero.'), 'ok');
  }

  // Get the highwater mark for this type so we can pick up where we left off.
  $highwater = springboard_dw_queue_legacy_get_highwater_mark($type);

  $records = springboard_dw_queue_legacy_get_records($type, $limit, $highwater, $idlist);

  $record_count = count($records);
  drush_log(dt('@count records found to process.', array('@count' => $record_count)), 'ok');
  if ($record_count < 1 && !$idlist) {
    watchdog('data_warehouse', 'Attempted to process %type records when there
     are :record_count remaining.', array('%type' => $type, ':record_count' => $record_count), WATCHDOG_WARNING);
  }

  // Break up all of our data so each process does not time out.
  $chunks = array_chunk($records, $batch_size);
  $operations = array();

  // For every chunk, assign some method to run on that chunk of data.
  $i = 0;
  foreach ($chunks as $chunk) {
    $i++;

    // The size of the chunk count should not be greater than the record
    // count but can be the size of the total record count.
    $chunk_count = ($i * $batch_size >= $record_count) ? $record_count:($i *
      $batch_size);

    $operations[] = array('springboard_dw_queue_legacy_' . $type,
                    array($chunk, 'details' => t(
                        '(Queued records @chunk of @count)',
                        array('@chunk' => $chunk_count, '@count' => $record_count)
                      ),
                      $idlist,
                    ),
    );
  }

  // Put all that information into our batch array.
  $batch = array(
    'operations' => $operations,
    'title' => t('Queue batch'),
    'init_message' => t('Initializing'),
    'error_message' => t('An error occurred'),
    'finished' => 'springboard_dw_queue_legacy_finished_batch',
  );

  // Get the batch process all ready!
  batch_set($batch);
  $batch =& batch_get();

  // Because we are doing this on the back-end, we set progressive to false.
  $batch['progressive'] = FALSE;

  // Start processing the batch operations.
  drush_backend_batch_process();
}

/**
 * Callback for finished batch.
 */
function springboard_dw_queue_legacy_finished_batch($success, $results, $operations) {
  drush_log(dt('Finished queueing'));
}

/**
 * Batch process callback for processing users.
 */
function springboard_dw_queue_legacy_contact($records, $operation_details, $idlist, &$context) {
  foreach ($records as $record) {
    // Create the user item in the queue if not warehoused.
    if (!springboard_dw_item_is_warehoused('contact', $record->id)) {

      // Take the account object and prep for contact mapping.
      $user_obj = entity_metadata_wrapper('user', $record->id);
      $user_data = springboard_dw_get_all_entity_properties($user_obj);

      // Map the user contact.
      $contact_item = springboard_dw_map_contact_item($record->id, $user_data);

      // Create the queue item to update the contact record.
      $queue = DrupalQueue::get("springboard_dw_export");
      $queue->createItem($contact_item);
      // Log the queue item.
      springboard_dw_log_queue_item($contact_item);
      // Mark the item as queued.
      springboard_dw_track_item_warehouse_queued('contact', $record->id);
    }
    else {
      drush_log('Attempted to queue contact ' . $record->id . ' but it is already warehoused', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('contact', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing donations.
 */
function springboard_dw_queue_legacy_donation($records, $operation_details, $idlist, &$context) {
  $queue = DrupalQueue::get("springboard_dw_export");
  foreach ($records as $record) {
    $donation = fundraiser_donation_get_donation($record->id);
    // Massage values into format springboard_dw_map_donation_item() expects.
    foreach ($donation->submission_data as $key => $value) {
      if (!array_key_exists($key, $donation->donation)) {
        $donation->donation[$key] = $value;
      }
    }
    // If $donation->data could not decrypted, just set the expected values to
    // empty string.
    if (!is_array($donation->data)) {
      $donation->data = array();
      $donation->data['payment_fields']['credit']['card_type'] = '';
      $donation->data['payment_fields']['credit']['card_number'] = '';
      $donation->data['payment_fields']['credit']['card_expiration_month'] = '';
      $donation->data['payment_fields']['credit']['card_expiration_year'] = '';
    }
    if (!springboard_dw_item_is_warehoused('donation', $record->id)) {
      $donation_item = springboard_dw_map_donation_item($donation);
      $queue->createItem($donation_item);
      springboard_dw_log_queue_item($donation_item);
      springboard_dw_track_item_warehouse_queued('donation', $record->id);
    }
    else {
      drush_log('Attempted to queue donation ' . $record->id . ' but it is already warehoused', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('donation', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing webforms.
 */
function springboard_dw_queue_legacy_form($records, $operation_details, $idlist, &$context) {
  foreach ($records as $record) {
    $node = node_load($record->id);
    // Get the Drupal data warehouse queue.
    $queue = DrupalQueue::get("springboard_dw_export");

    // Create the form item in the queue if not warehoused.
    if (!springboard_dw_item_is_warehoused('form', $record->id)) {
      $form_item = springboard_dw_map_form_item($node);
      $queue->createItem($form_item);
      springboard_dw_log_queue_item($form_item);
      springboard_dw_track_item_warehouse_queued('form', $record->id);
    }
    else {
      drush_log('Attempted to queue form ' . $record->id . ' but it is already warehoused.', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('form', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing payments
 */
function springboard_dw_queue_legacy_payment($records, $operation_details, $idlist, &$context) {
  foreach ($records as $record) {
    $queue = DrupalQueue::get("springboard_dw_export");
    // Create the form item in the queue if not warehoused.
    if (!springboard_dw_item_is_warehoused('payment', $record->id)) {
      $donation['donation_id'] = $record->id;
      $payment['uid'] = $record->uid;
      $payment['payment_method'] = $record->payment_method;
      $payment['instance_id'] = $record->instance_id;
      $payment['remote_id'] = $record->remote_id; // the id returned from the gateway
      $payment['amount'] = $record->amount;
      $payment['currency_code'] = $record->currency_code;
      $payment['status'] = $record->status;
      $payment['message'] = $record->message;
      $payment['remote_status'] = $record->remote_status;
      $payment['created'] = $record->created;
      $payment['changed'] = $record->changed;
      $payment_item = springboard_dw_map_payment_item($payment, $donation);
      $queue->createItem($payment_item);
      springboard_dw_log_queue_item($payment_item);
      springboard_dw_track_item_warehouse_queued('payment', $record->id);
    }
    else {
      drush_log('Attempted to queue payment ' . $record->id . ' but it is already warehoused', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('payment', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing webform submissions.
 *
 * Submissions for content types 'donation_form', 'springboard_petition',
 * sba_social_action', 'sba_message_action' should not be passed to this
 * function per the DW schema.
 */
function springboard_dw_queue_legacy_submission($records, $operation_details, $idlist, &$context) {
  // Get the Drupal data warehouse queue.
  $queue = DrupalQueue::get("springboard_dw_export");
  module_load_include('inc', 'webform', 'includes/webform.submissions');
  foreach ($records as $record) {
    $node = node_load($record->nid);
    $submission = webform_get_submission($record->nid, $record->id);
    // If submission is false, the webform_submitted_data table was probably
    // cleared out at some point. So, let's populate a $submission object with
    // the data we have available from the webform_submissions table.
    if (!$submission) {
      $submission = new stdClass();
      $submission->sid = $record->id;
      $submission->nid = $record->nid;
      $submission->uid = $record->uid;
      $submission->submitted = $record->created;
      $submission->remote_addr = $record->remote_addr;
    }
    if (is_array($submission->data)) {
      $submitted_data = array();
      // Put the submission data in key/value pair format.
      foreach ($submission->data as $cid => $submittted_value) {
        if (array_key_exists($cid, $node->webform['components'])) {
          $submitted_data[$node->webform['components'][$cid]['form_key']] = $submission->data[$cid]['value'][0];
        }
      }
    }
    if (!springboard_dw_item_is_warehoused('submission', $record->id)) {
      $submission_item = springboard_dw_map_submission_item($submission, $submitted_data);
      // Create the queue item.
      $queue->createItem($submission_item);
      springboard_dw_log_queue_item($submission_item);
      springboard_dw_track_item_warehouse_queued('submission', $record->id);
    }
    else {
      drush_log('Attempted to queue submission ' . $record->nid . ' but it is already warehoused', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('submission', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing actions.
 */
function springboard_dw_queue_legacy_action($records, $operation_details, $idlist, &$context) {
  // Get the Drupal data warehouse queue.
  $queue = DrupalQueue::get("springboard_dw_export");

  foreach ($records as $record) {
    $action = new stdClass();
    $action->node = node_load($record->nid);
    $action->webform = springboard_dw_webform_submission_data_keyed($record->nid, $record->sid);
    $action->sid = $record->sid;
    $action->uid = $record->uid;
    $action->ip_address = $record->remote_addr;

    // Queue message action and social action.
    if ($record->type == 'sba_message_action' || $record->type == 'sba_social_action') {
      // Create the Advocacy action item in the queue if not warehoused.
      if (!springboard_dw_item_is_warehoused('advocacy action', $record->id)) {
        $action_item = springboard_dw_map_advocacy_action_item($action);
        $queue->createItem($action_item);
        springboard_dw_log_queue_item($action_item);
        springboard_dw_track_item_warehouse_queued('advocacy action', $record->id);
      }
      else {
        drush_log('Attempted to queue action ' . $record->id . ' but it is already warehoused', 'warning');
      }
    }
    // Queue petition.
    elseif ($record->type == 'springboard_petition') {
      if (!springboard_dw_item_is_warehoused('petition', $record->id)) {
        $action_item = springboard_dw_map_petition_item($action);
        $queue->createItem($action_item);
        springboard_dw_log_queue_item($action_item);
        springboard_dw_track_item_warehouse_queued('petition', $record->id);
      }
      else {
        drush_log('Attempted to queue petition ' . $record->id . ' but it is already warehoused', 'warning');
      }
    }
    else {
      drush_log('Record type ' . $record->type . ' on record ' . $record->id . ' not recognized', 'error');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('action', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing petitions.
 */
function springboard_dw_queue_legacy_petition_message($records, $operation_details, $idlist, &$context) {
  // Get the Drupal data warehouse queue.
  $queue = DrupalQueue::get("springboard_dw_export");
  foreach ($records as $record) {
    // Create the Advocacy action item in the queue if not warehoused.
    if (!springboard_dw_item_is_warehoused('advocacy petition message', $record->id)) {
      $action = springboard_dw_queue_legacy_build_action($record);
      $petition_item = springboard_dw_map_petition_submission_item($action, '');
      $queue->createItem($petition_item);
      springboard_dw_log_queue_item($petition_item);
      springboard_dw_track_item_warehouse_queued('advocacy petition message', $record->id);
    }
    else {
      drush_log('Attempted to queue petition message' . $record->id . ' but it is already warehoused', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('petition_message', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing advocacy messages.
 */
function springboard_dw_queue_legacy_message($messages, $operation_details, $idlist, &$context) {
  // Get the Drupal data warehouse queue.
  $queue = DrupalQueue::get("springboard_dw_export");
  foreach ($messages as $message) {
    // Create the Advocacy action item in the queue if not warehoused.
    if (!springboard_dw_item_is_warehoused('advocacy message', $message['_id']['$id'])) {
      $message_item = array();
      $message_item['queue_type'] = 'advocacy message';
      $message_item['message_id'] = $message['_id']['$id'];
      $message_item['action_id'] = $message['alertId'];
      $tracking_response = springboard_dw_queue_legacy_get_endpoint($message['_id']['$id'], 'warehouse/track');
      if (isset($tracking_response['warehoused']) && $tracking_response['warehoused'] == TRUE) {
        $queue->createItem($message_item);
        springboard_dw_log_queue_item($message_item);
        springboard_dw_track_item_warehouse_queued('advocacy message', $message['_id']['$id']);
      }
      else {
        drush_log('Attempted to track message ' . $message['_id']['$id'] . ' at warehouse/track but there was a problem. Skipping it this run.', 'warning');
      }
    }
    else {
      drush_log('Attempted to queue message ' . $message['_id']['$id'] . ' but it is already warehoused', 'warning');
    }
  }
  $context['message'] = $operation_details;
}

/**
 * Batch process callback for processing salesforce_sync items.
 */
function springboard_dw_queue_legacy_salesforce_sync($records, $operation_details, $idlist, &$context) {
  foreach ($records as $record) {
    if (!springboard_dw_item_is_warehoused('salesforce sync', $record->id)) {
      $salesforce_sync_array = (array) $record;
      $salesforce_sync_item = springboard_dw_map_sync_map_item($salesforce_sync_array);
      $queue = DrupalQueue::get("springboard_dw_export");
      $queue->createItem($salesforce_sync_item);
      springboard_dw_log_queue_item($salesforce_sync_item);
      springboard_dw_track_item_warehouse_queued('salesforce sync', $record->id);
    }
    else {
      drush_log('Attempted to queue salesforce_sync ' . $record->id . ' but it is already warehoused', 'warning');
    }
    // Set highwater mark.
    springboard_dw_queue_legacy_set_highwater_mark('salesforce_sync', $record->id, $idlist);
  }
  $context['message'] = $operation_details;
}

/**
 * Builds action object.
 *
 *  Build the action object by re-creating the data that is available during
 *  hook_advocacy_success().
 */
function springboard_dw_queue_legacy_build_action($submission) {
  $node = node_load($submission->nid);
  $webform_submission_data = springboard_dw_webform_submission_data_keyed($node->nid, $submission->sid);
  $user = user_load($submission->uid);
  $action = new stdClass();
  $action->node = $node;
  $action->contact = $user;
  // @TODO how to populate $data? What even goes in there?
  $action->data = $data;
  $action->webform = $webform_submission_data;
  $action->sid = $sid;
  return $action;
}

/**
 * Get list of advocacy message node IDs.
 *
 * @return array
 *   Array of all message values or an empty array.
 */
function springboard_dw_queue_legacy_get_advocacy_messages($limit = FALSE) {
  // Get nids of all sba_message_action nodes.
  $advocacy_action_records = db_query("SELECT nid FROM {node} WHERE type = 'sba_message_action' OR type = 'sba_social_action' ORDER BY created ASC", array(), array('target' => 'slave'))->fetchAll();
  $messages = array();
  foreach ($advocacy_action_records as $advocacy_action_record) {
    $node = node_load($advocacy_action_record->nid);
    // Get count of remaining messages for this advocacy_id.
    $metrics = springboard_dw_queue_legacy_get_endpoint($node->advocacy_id, 'warehouse/metrics');
    if (!isset($metrics['message'])) {
      drush_log(dt('There was a problem retrieving message count for action: @action_id', array('@action_id' => $node->advocacy_id)), 'error');
    }
    else {
      drush_log($metrics['message'], 'ok');
      // If count of warehosued messages is less than total messages.
      if ($metrics['warehousedMessages'] < $metrics['totalCount']) {
        $api_params = ($limit !== FALSE) ? array('limit' => $limit) : array();
        $messages = springboard_dw_queue_legacy_get_endpoint($node->advocacy_id, 'warehouse/untracked', $api_params);
        if ($messages !== FALSE) {
          // We found messages to queue, so break out of this foreach. To get
          // the rest of the messages, keep running this drush commend.
          break;
        }
        else {
          drush_log(dt('There was a problem retrieving untracked messages for action: @action_id', array('@action_id' => $node->advocacy_id)), 'error');
        }
      }
    }
  }
  return isset($messages) ? $messages : array();
}

/**
 * GET a single page of untracked messages.
 *
 * @return mixed
 *   Return FALSE on failure or an array of message data for a specified
 *   pagination offset.
 *
 * @todo add limit param.
 */
function springboard_dw_queue_legacy_get_endpoint($id, $endpoint, $additional_params = array()) {
  // Composer Vendor autoload.
  if (function_exists('drush_autoload')) {
    drush_autoload("sites/all/libraries/springboard_composer/vendor/autoload.php");
  }
  else {
    require_once "sites/all/libraries/springboard_composer/vendor/autoload.php";
  }

  // Load Guzzle client.
  $guzzle = new Client(['http_errors' => FALSE]);
  $params = array(
    'query' => array(
      'access_token' => variable_get('springboard_dw_advocacy_authentication_token'),
    ),
  );
  $params['query'] += $additional_params;
  $endpoint = variable_get('springboard_dw_advocacy_api_endpoint') . '/api/v1/messages/' . $id . '/' . $endpoint;
  $response = $guzzle->request('GET', $endpoint, $params);
  $status_code = $response->getStatusCode();
  if ($response->getStatusCode() == 200) {
    $body = $response->getBody();
    return json_decode($body->getContents(), TRUE);
  }
  else {
    drush_log(dt('GET response code: @code', array('@code' => $response->getStatusCode())), 'error');
    return FALSE;
  }
}

/**
 *
 */
function springboard_dw_queue_legacy_get_records($type, $limit = FALSE, $highwater = 0, $idlist = FALSE) {
  // Populate list of records to operate on.
  switch ($type) {
    case 'contact':
      $query = db_select('users', 'u', array('target' => 'slave'));
      $query->addField('u', 'uid', 'id');
      $query->addField('u', 'created', 'created');
      $query->orderBy('uid', 'ASC');
      $query->condition('uid', 0, '<>');
      if ($idlist) {
        $query->condition('uid', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('uid', $highwater, '>');
      }
      break;

    case 'donation':
      $query = db_select('fundraiser_donation', 'f', array('target' => 'slave'));
      $query->addField('f', 'did', 'id');
      $query->addField('f', 'nid');
      $query->addField('f', 'created');
      $query->orderBy('did', 'ASC');
      if ($idlist) {
        $query->condition('did', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('did', $highwater, '>');
      }
      break;

    case 'action':
      $query = db_select('webform_submissions', 's', array('target' => 'slave'));
      $query->addField('s', 'sid', 'id');
      $query->addField('s', 'nid');
      $query->addField('s', 'uid');
      $query->addField('s', 'remote_addr');
      $query->addField('s', 'submitted', 'created');
      $query->addField('n', 'type');
      $query->join('node', 'n', 's.nid = n.nid');
      $query->condition('n.type', array('sba_message_action', 'sba_social_action', 'springboard_petition'), 'IN');
      $query->orderBy('sid', 'ASC');
      if ($idlist) {
        $query->condition('sid', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('sid', $highwater, '>');
      }
      break;

    case 'message':
      $records = springboard_dw_queue_legacy_get_advocacy_messages($limit);
      break;

    case 'petition_message':
      $query = db_select('webform_submissions', 's', array('target' => 'slave'));
      $query->addField('s', 'sid', 'id');
      $query->addField('s', 'nid');
      $query->addField('s', 'uid');
      $query->addField('s', 'submitted', 'created');
      $query->join('node', 'n', 's.nid = n.nid');
      $query->condition('n.type', 'springboard_petition', '=');
      $query->orderBy('sid', 'ASC');
      if ($idlist) {
        $query->condition('sid', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('sid', $highwater, '>');
      }
      break;

    case 'form':
      $query = db_select('node', 'n', array('target' => 'slave'));
      $query->addField('n', 'nid', 'id');
      $query->addField('n', 'created');
      $query->orderBy('n.nid', 'ASC');
      $query->join('webform', 'w', 'n.nid = w.nid');
      if ($idlist) {
        $query->condition('n.nid', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('n.nid', $highwater, '>');
      }
      break;

    case 'submission':
      $excluded_types = springboard_dw_get_donation_types();
      $excluded_types[] = 'springboard_petition';
      $excluded_types[] = 'sba_message_action';
      $excluded_types[] = 'sba_social_action';
      $query = db_select('webform_submissions', 's', array('target' => 'slave'));
      $query->addField('s', 'sid', 'id');
      $query->addField('s', 'nid');
      $query->addField('s', 'uid');
      $query->addField('s', 'remote_addr');
      $query->addField('s', 'submitted', 'created');
      $query->join('node', 'n', 's.nid = n.nid');
      $query->condition('n.type', $excluded_types, 'NOT IN');
      $query->orderBy('sid', 'ASC');
      if ($idlist) {
        $query->condition('sid', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('sid', $highwater, '>');
      }
      break;

    case 'payment':
      $query = db_select('commerce_payment_transaction', 'c', array('target' => 'slave'));
      $query->addField('c', 'transaction_id', 'id');
      $query->addField('c', 'order_id', 'donation_id');
      $query->addField('c', 'remote_id');
      $query->addField('c', 'uid');
      $query->addField('c', 'payment_method');
      $query->addField('c', 'instance_id');
      $query->addField('c', 'currency_code');
      $query->addField('c', 'amount');
      $query->addField('c', 'status');
      $query->addField('c', 'message');
      $query->addField('c', 'remote_status');
      $query->addField('c', 'created');
      $query->addField('c', 'changed');
      $query->orderBy('transaction_id', 'ASC');
      if ($idlist) {
        $query->condition('transaction_id', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('transaction_id', $highwater, '>');
      }
      break;

    case 'salesforce_sync':
      $query = db_select('salesforce_sync_map', 's', array('target' => 'slave'));
      $query->addField('s', 'rmid', 'id');
      $query->addField('s', 'mid');
      $query->addField('s', 'sfid');
      $query->addField('s', 'drupal_id');
      $query->addField('s', 'module');
      $query->addField('s', 'delta');
      $query->addField('s', 'object_type');
      $or = db_or();
      $or->condition('delta', array('user', 'sba_message_action', 'sba_social_action', 'springboard_petition', 'donation'), 'IN');
      $or->condition('object_type', 'sb_Actions_Taken__c', '=');
      $query->condition($or);
      $query->condition('sfid', '', '<>');
      $query->orderBy('rmid', 'ASC');

      if ($idlist) {
        $query->condition('rmid', $idlist, 'IN');
      }
      elseif ($highwater) {
        $query->condition('rmid', $highwater, '>');
      }
      break;

    default:
      return drush_set_error(dt('"@type" is not a valid record type. Nothing was processed.', array('@type' => $type)));
  }
  if (isset($query)) {
    if ($limit) {
      $query->range(0, $limit);
    }
    $records = $query->execute()->fetchAll();
  }
  return $records;
}

/**
 * Find records that have not been added to salesforce_sync_map.
 */
function springboard_dw_queue_legacy_get_missing_records($type) {
  // Populate list of records missing from springboard_data_warehouse.
  switch ($type) {
    case 'contact':
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'users' as table_name, uid as id
                              FROM
                              {users} u
                              WHERE  (uid <> 0)
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'contact'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'donation':
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'fundraiser_donation' as table_name, did as id
                              FROM
                              {fundraiser_donation}
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'donation'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'action':
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'webform_submissions' as table_name, sid as id
                              FROM {webform_submissions} ws
                              INNER JOIN {node} n on n.nid = ws.nid
                              WHERE n.type IN ('sba_message_action', 'sba_social_action', 'springboard_petition')
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type IN ('advocacy action', 'petition')
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'message':
      $records = springboard_dw_queue_legacy_get_advocacy_messages();
      break;

    case 'petition_message':
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'webform_submissions' as table_name, sid as id
                              FROM {webform_submissions} ws
                              INNER JOIN {node} n on n.nid = ws.nid
                              WHERE n.type = 'springboard_petition'
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'petition_message'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'form':
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'node' as table_name, n.nid as id
                              FROM {node} n
                              INNER JOIN {webform} w on n.nid = w.nid
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'form'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'submission':
      $excluded_types = springboard_dw_get_donation_types();
      $excluded_types[] = 'springboard_petition';
      $excluded_types[] = 'sba_message_action';
      $excluded_types[] = 'sba_social_action';
      $excluded_string = "'" . implode("','", $excluded_types) . "'";
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'webform_submissions' as table_name, sid as id
                              FROM {webform_submissions} ws
                              INNER JOIN {node} n on n.nid = ws.nid
                              WHERE n.type NOT IN (" . $excluded_string . ")
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'submission'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'payment':

      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'commerce_payment_transaction' as table_name, transaction_id as id
                              FROM {commerce_payment_transaction}
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'payment'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    case 'salesforce_sync':
      $result = db_query("SELECT table_name, id
                          FROM
                          (
                            SELECT 'salesforce_sync_map' as table_name, rmid as id
                              FROM {salesforce_sync_map}
                              WHERE (delta IN ('user', 'sba_message_action', 'sba_social_action', 'springboard_petition', 'donation')
                              OR object_type = 'sb_Actions_Taken__c') AND sfid <> ''
                            UNION ALL
                            SELECT 'springboard_data_warehouse' as table_name, drupal_id as id
                              FROM {springboard_data_warehouse}
                              WHERE type = 'salesforce sync'
                          ) tmp
                          GROUP BY id
                          HAVING COUNT(*) = 1")->fetchAll();
      break;

    default:
      return drush_set_error(dt('"@type" is not a valid record type. Nothing was processed.', array('@type' => $type)));
  }

  return $result;
}

function springboard_dw_queue_legacy_get_dw_by_type($type) {
  $query = db_select('springboard_data_warehouse', 's', array('target' => 'slave'));
  $query->addField('s', 'drupal_id', 'id');
  $query->condition('type', $type, '=');
  $records = $query->execute()->fetchCol();
  return $records;
}
