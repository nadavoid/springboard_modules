<?php

/**
 * @file
 * Drush commands for Webform User.
 */

/**
 * Implements hook_drush_command().
 */
function webform_user_drush_command() {
  $items = array();

  $items['webform-create-missing-users'] = array(
    'callback' => 'webform_user_create_missing_users',
    'description' => 'Create missing users from webform submissions.',
  );

  return $items;
}

/**
 * Cron callback function. Creates missing users from webform submissions.
 */
function webform_user_create_missing_users($node_id) {
  // Create a log table.
  webform_user_create_missing_users_create_log_table();

  // Get all webform submission IDs with an empty user ID for the given node ID.
  $query = 'SELECT `sid`
    FROM `webform_submissions`
    WHERE `nid` = :nid
    AND `uid` = :uid
    ORDER BY `submitted` ASC';

  $result = db_query($query, array(
    ':nid' => $node_id,
    ':uid' => 0,
  ));

  $submission_ids = array();
  foreach ($result as $record) {
    $submission_ids[] = $record->sid;
  }

  if (!empty($submission_ids)) {
    $chunks = array_chunk($submission_ids, 10);
    $operations = array();
    $chunk_count = count($chunks);
    $i = 0;
    foreach ($chunks as $chunk) {
      $i++;
      $operations[] = array('webform_user_create_missing_users_batch_process',
        array(
          $chunk,
          $node_id,
          'details' => t('(Processing chunk @chunk of @count)', array('@chunk' => $i, '@count' => $chunk_count)))
      );
    }

    $batch = array(
      'operations' => $operations,
      'title' => t('Creating missing webform users'),
      'init_message' => t('Initializing'),
      'error_message' => t('An error occurred'),
      'finished' => 'webform_user_create_missing_users_batch_process_complete'
    );

    batch_set($batch);
    $batch = & batch_get();
    $batch['progressive'] = FALSE;

    drush_backend_batch_process();
  }
}

function webform_user_create_missing_users_batch_process($chunk, $node_id, $operation_details, &$context) {
  module_load_include('inc', 'webform', 'includes/webform.submissions');

  $context['message'] = $operation_details;

  // Get all webform submissions for the current set of submission IDs.
  $query = 'SELECT `sd`.`sid`, `cm`.`map_id`, `sd`.`data`
    FROM `webform_submissions` `s`
    RIGHT JOIN `webform_submitted_data` `sd` ON `sd`.`sid` = `s`.`sid`
    RIGHT JOIN `webform_user_component_map` `cm` ON `cm`.`nid` = `sd`.`nid` AND `cm`.`cid` = `sd`.`cid`
    WHERE `s`.`sid` IN (:submission_ids) ORDER BY `s`.`sid` ASC;';

  $result = db_query($query, array(
    ':submission_ids' => implode(',', $chunk),
  ));

  $submissions = array();

  foreach ($result as $record) {
    if (!isset($submissions[$record->sid])) {
      $submissions[$record->sid] = array();
    }

    $submissions[$record->sid][$record->map_id] = $record->data;
  }

  // Create users from the webform submissions.
  foreach ($submissions as $sid => $submission) {
    $account = user_load_by_mail($submission['mail']);
    
    // Remove empty string values from submission to avoid datatype exceptions
    $submission = array_filter($submission, 'strlen');
    if (empty($account)) {
      $user_fields = array(
        'name' => $submission['mail'],
        'mail' => $submission['mail'],
        'init' => $submission['mail'],
        'pass' => user_password(8),
        'status' => 1,
      );

      $account = user_save('', $user_fields);
    }

    _webform_user_save_profile_map($account->uid, $submission);

    $submission = webform_get_submission($node_id, $sid);
    $submission->uid = $account->uid;
    // Skip the current user check; this is an offline user creation.
    $submission->webform_user_skip_user_check = TRUE;

    $node = node_load($node_id);

    // Update the webform submission with the new user ID.
    webform_submission_update($node, $submission);

    // Log the new user creation.
    try {
      db_insert('webform_user_create_missing_users_log')->fields(array(
        'nid' => $node_id,
        'uid' => $account->uid,
        'timestamp' => time(),
      ))->execute();
    } catch (PDOException $e) {
      print $e->getMessage() . "\n";
    }
  }
}

function webform_user_create_missing_users_batch_process_complete($success, $results, $operations) {
  print t('Webform user creation complete.');
}

function webform_user_create_missing_users_create_log_table() {
  if (!db_table_exists('webform_user_create_missing_users_log')) {
    $schema = array(
      'fields' => array(
        'nid' => array(
          'description' => 'The node ID of the webform.',
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'default' => 0,
        ),
        'uid' => array(
          'description' => 'The ID of the user created.',
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'default' => 0,
        ),
        'timestamp' => array(
          'description' => 'The timestamp when the user was created.',
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'default' => 0,
        ),
      ),
      'primary key' => array('nid', 'uid'),
    );

    db_create_table('webform_user_create_missing_users_log', $schema);
  }
}
