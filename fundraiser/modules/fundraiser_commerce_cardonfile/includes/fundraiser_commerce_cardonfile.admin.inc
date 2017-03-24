<?php
/**
 * @file
 * Fundraiser Commerce Cardonfile admin functions.
 */

const FUNDRAISER_COMMERCE_CARDONFILE_DONATION_ENTRIES_THRESHOLD = 50;
const FUNDRAISER_COMMERCE_CARDONFILE_CARD_ENTRIES_THRESHOLD = 50;
const FUNDRAISER_COMMERCE_CARDONFILE_DONATIONS_LEFT_STATIC_NAME = 'fundraiser_commerce_cardonfile_donations_left';
const FUNDRAISER_COMMERCE_CARDONFILE_CARDS_LEFT_STATIC_NAME = 'fundraiser_commerce_cardonfile_cards_left';
const FUNDRAISER_COMMERCE_CARDONFILE_CARDS_STATIC_NAME = 'fundraiser_commerce_cardonfile_cards';

/**
 * Admin settings form.
 */
function fundraiser_commerce_cardonfile_admin_settings($form, $form_state) {
  $number_unupdated_donations = _fundraiser_commerce_cardonfile_number_of_unupdated_donations();

  if ($number_unupdated_donations > 0) {
    $options = array(
      '!n' => '<strong>' . $number_unupdated_donations . '</strong>',
      '@message' => 'You can either use the following button to process all donations, or use this drush command:',
      '!command' => '<code>drush fccdp</code>.<br/>',
    );
    if ($number_unupdated_donations >= FUNDRAISER_COMMERCE_CARDONFILE_DONATION_ENTRIES_THRESHOLD) {
      $options['@message'] = 'Since the number of donations without associated card ids is greater than the recommmended threshold for processing all donations, it is recommmended you process a set of donations at a time. You can either use the following buttons to process a set number of donations, all donations, or use the following drush command:';
    }
    $message = 'There are currently !n donations without associated card ids. @message !command';
    if ($number_unupdated_donations == 1) {
      $message = 'There is currently !n donation without an associated card id. @message !command';
    }
    $form['unupdated_donation_stats'] = array(
      '#type' => 'markup',
      '#markup' => t($message, $options),
    );
    if ($number_unupdated_donations >= FUNDRAISER_COMMERCE_CARDONFILE_DONATION_ENTRIES_THRESHOLD) {
      $form['process_donation_set'] = array(
        '#type' => 'submit',
        '#value' => t('Process @n donations', array('@n' => FUNDRAISER_COMMERCE_CARDONFILE_DONATION_ENTRIES_THRESHOLD)),
        '#submit' => array('_fundraiser_commerce_cardonfile_process_limited_donations'),
      );
    }
    if ($number_unupdated_donations > 0) {
      $form['process_donation_all'] = array(
        '#type' => 'submit',
        '#value' => t('Process all donations'),
        '#submit' => array('_fundraiser_commerce_cardonfile_process_donations'),
      );
    }
  }

  $form['fundraiser_commerce_cardonfile_patch_cardonfile'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable separate billing addresses for unique cards'),
    '#description' => t("This will enable functionality to separate the user's regular address (stored in the user table) from their stored card's billing address, a feature the commerce_cardonfile module does not yet currently support."),
    '#default_value' => variable_get('fundraiser_commerce_cardonfile_patch_cardonfile', TRUE),
  );

  if (variable_get('fundraiser_commerce_cardonfile_patch_cardonfile', TRUE)) {
    $number_unupdated_cards = _fundraiser_commerce_cardonfile_number_of_unupdated_cards();
    if ($number_unupdated_cards > 0) {
      $options = array(
        '!n' => '<strong>' . $number_unupdated_cards . '</strong>',
        '@message' => 'You can either use the following button to process all donations, or use this drush command:',
        '!command' => '<code>drush fcccp</code>.<br/>',
      );
      if ($number_unupdated_cards >= FUNDRAISER_COMMERCE_CARDONFILE_CARD_ENTRIES_THRESHOLD) {
        $options['@message'] = 'Since the number of cards without associated addresses is greater than the recommmended threshold for processing all cards, it is recommmended you process a set of cards at a time. You can either use the following buttons to process a set number of cards, all cards, or use the following drush command:';
      }
      $message = 'There are currently !n cards without associated addresses. @message !command';
      if ($number_unupdated_cards == 1) {
        $message = 'There is currently !n card without an associated address id. @message !command';
      }
      $form['unupdated_card_stats'] = array(
        '#type' => 'markup',
        '#markup' => t($message, $options),
      );
    }

    if ($number_unupdated_cards >= FUNDRAISER_COMMERCE_CARDONFILE_CARD_ENTRIES_THRESHOLD) {
      $form['process_card_set'] = array(
        '#type' => 'submit',
        '#value' => t('Process @n cards', array('@n' => FUNDRAISER_COMMERCE_CARDONFILE_CARD_ENTRIES_THRESHOLD)),
        '#submit' => array('_fundraiser_commerce_cardonfile_process_limited_cards'),
      );
    }
    if ($number_unupdated_cards > 0) {
      $form['process_card_all'] = array(
        '#type' => 'submit',
        '#value' => t('Process all cards'),
        '#submit' => array('_fundraiser_commerce_cardonfile_process_cards'),
      );
    }
  }

  return system_settings_form($form);
}

/**
 * Submission handler, processes donations to update their card id values.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state array.
 * @param int $limit
 *   The number of donations to process.
 *
 * @see fundraiser_commerce_cardonfile_admin_settings()
 */
function _fundraiser_commerce_cardonfile_process_limited_donations($form, $form_state, $limit = FUNDRAISER_COMMERCE_CARDONFILE_DONATION_ENTRIES_THRESHOLD) {
  _fundraiser_commerce_cardonfile_process_donations($form, $form_state, $limit);
}

/**
 * Submission handler, processes donations to update their card id values.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state array.
 *
 * @see fundraiser_commerce_cardonfile_admin_settings()
 */
function _fundraiser_commerce_cardonfile_process_donations($form, $form_state, $limit = FUNDRAISER_COMMERCE_CARDONFILE_DONATION_ENTRIES_THRESHOLD) {
  $batch = array(
    'operations' => array(
      array(
        '_fundraiser_commerce_cardonfile_update_donation_cardids',
        array($limit, $limit),
      ),
    ),
    'finished' => '_fundraiser_commerce_cardonfile_processed_donations',
    'title' => t('Processing Donations'),
    'init_message' => t('Donation processing is starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Donation processing has encountered an error.'),
    'file' => drupal_get_path('module', 'fundraiser_commerce_cardonfile') . '/includes/fundraiser_commerce_cardonfile.admin.inc',
  );
  batch_set($batch);
}

/**
 * Batch processor to process donations with null card ids.
 */
function _fundraiser_commerce_cardonfile_update_donation_cardids($limit, $max_limit = NULL, &$context = array()) {
  if (!is_numeric($limit)) {
    drupal_set_message(t('The limit was not a valid number, in "_fundraiser_commerce_cardonfile_update_donation_cardids".'), 'warning');
    return;
  }

  if (!empty($max_limit) && $limit > $max_limit) {
    drupal_set_message(t('The max limit cannot exceed the limit, in "_fundraiser_commerce_cardonfile_update_donation_cardids".'), 'warning');
    return;
  }

  $sandbox = $context['sandbox'];
  $cards = &drupal_static(FUNDRAISER_COMMERCE_CARDONFILE_CARDS_STATIC_NAME);

  if (!isset($sandbox['progress'])) {
    // The count of donations visited so far.
    $sandbox['progress'] = 0;
    // Total donations that must be visited. Select donations that have a status
    // of "payment_received" but no card_id. Only selecting donations with this
    // status because any donations that will be processed later should have
    // their card ids set via
    // `fundraiser_commerce_cardonfile_fundraiser_donation_post_update`.
    $sandbox['max'] = _fundraiser_commerce_cardonfile_number_of_unupdated_donations();
    // A place to store messages during the run.
    $sandbox['messages'] = array();
    // Last donation read via the query.
    $sandbox['current_donation'] = -1;
  }

  // Retrieve the next group of donations to update.
  $result = db_select('fundraiser_donation', 'fd')
    ->distinct()
    ->fields('fd', array('did', 'uid'))
    ->orderBy('fd.did', 'ASC')
    ->condition('fd.did', $sandbox['current_donation'], '>')
    ->condition('fd.status', 'payment_received', '=')
    ->isNull('card_id');

  if (module_exists('fundraiser_sustainers')) {
    if (empty($master_sustainer_donation_ids)) {
      $master_sustainer_donation_ids = db_select('fundraiser_sustainers', 'fs')->fields('fs', array('master_did'))->groupBy('master_did');
    }
    if (empty($sustainer_donation_ids)) {
      $sustainer_donation_ids = db_select('fundraiser_sustainers', 'fs')->fields('fs', array('did'));
    }
    $or = db_or();
    $or->condition('fd.did', $master_sustainer_donation_ids, 'IN');
    $or->condition('fd.did', $sustainer_donation_ids, 'NOT IN');
    $result = $result->condition($or);
  }

  $result = $result->range(0, $limit)
    ->execute()
    ->fetchAll();

  foreach ($result as $row) {
    $sandbox['progress']++;
    $sandbox['current_donation'] = $row->did;

    $card_id = NULL;

    if (empty($cards[$row->uid])) {
      $user_cards = commerce_cardonfile_load_multiple_by_uid($row->uid);
    }
    else {
      $user_cards = $cards[$row->uid];
    }
    $cards_n = count($user_cards);

    if ($cards_n == 1) {
      // If the user only has one card, we can just set it on the donation
      // record right away.
      $card = array_shift($user_cards);
      $card_id = $card->card_id;
    }
    else {
      // Otherwise we'll need to grab the card_id from the order record.
      $order = commerce_order_load($row->did);
      $data = _fundraiser_commerce_order_data_retrieve($order->data['fundraiser_commerce']);
      if (!emptY($data['cardonfile'])) {
        $card_id = $data['cardonfile'];
      }
    }

    if (empty($card_id)) {
      // Worst case time-wise, since the card_id wasn't set in the order for
      // some reason, we'll need to try a last-ditch effort to load the donation
      // and get the card_id from a fundraiser_commerce function.
      $donation = fundraiser_donation_get_donation($row->did);
      $card = _fundraiser_commerce_donation_cardonfile_card($donation);
      if (!empty($card)) {
        $card_id = $card->card_id;
      }
    }

    if (empty($card_id) || !is_numeric($card_id)) {
      $message = 'No card id was found for donation @did while updating donation information.';
      $params = array(
        '@did' => $row->did,
      );
      drupal_set_message(t($message, $params), 'warning');
      // Nothing found, we'll have to skip this donation.
      watchdog('fundraiser', $message, $params, WATCHDOG_NOTICE);
      continue;
    }

    db_query('UPDATE {fundraiser_donation} SET card_id = :card_id WHERE did = :did', array(
      ':card_id' => $card_id,
      ':did' => $row->did,
    ));
  }

  // Set the "finished" status, to tell batch engine whether this function
  // needs to run again. If you set a float, this will indicate the progress
  // of the batch so the progress bar will update.
  $context['finished'] = ($sandbox['progress'] >= $sandbox['max']) ? TRUE : ($sandbox['progress'] / $sandbox['max']);
}

/**
 * Donation processing batch 'finished' callback.
 */
function _fundraiser_commerce_cardonfile_processed_donations($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $message = t('@count donations successfully processed.', array('@count' => count($results)));
    $message .= theme('item_list', array('items' => $results));
    drupal_set_message($message);
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE),
    ));
    drupal_set_message($message, 'error');
  }
}

/**
 * Submission handler, processes cards to add their address values.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state array.
 * @param int $limit
 *   The number of cards to process.
 *
 * @see fundraiser_commerce_cardonfile_admin_settings()
 */
function _fundraiser_commerce_cardonfile_process_limited_cards($form, $form_state, $limit = FUNDRAISER_COMMERCE_CARDONFILE_CARD_ENTRIES_THRESHOLD) {
  _fundraiser_commerce_cardonfile_process_cards($form, $form_state, $limit);
}

/**
 * Submission handler, processes donations to update their card id values.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state array.
 *
 * @see fundraiser_commerce_cardonfile_admin_settings()
 */
function _fundraiser_commerce_cardonfile_process_cards($form, $form_state, $limit = FUNDRAISER_COMMERCE_CARDONFILE_CARD_ENTRIES_THRESHOLD) {
  $batch = array(
    'operations' => array(
      array(
        '_fundraiser_commerce_cardonfile_update_card_addresses',
        array($limit, $limit),
      ),
    ),
    'finished' => '_fundraiser_commerce_cardonfile_processed_cards',
    'title' => t('Processing Cards'),
    'init_message' => t('Card processing is starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Card processing has encountered an error.'),
    'file' => drupal_get_path('module', 'fundraiser_commerce_cardonfile') . '/includes/fundraiser_commerce_cardonfile.admin.inc',
  );
  batch_set($batch);
}

/**
 * Batch processor to process donations with null card ids.
 */
function _fundraiser_commerce_cardonfile_update_card_addresses($limit, $max_limit = NULL, &$context = array()) {
  if (!is_numeric($limit)) {
    drupal_set_message(t('The limit was not a valid number, in "_fundraiser_commerce_cardonfile_update_card_addresses".'), 'warning');
    return;
  }

  if (!empty($max_limit) && $limit > $max_limit) {
    drupal_set_message(t('The max limit cannot exceed the limit, in "_fundraiser_commerce_cardonfile_update_card_addresses".'), 'warning');
    return;
  }

  $sandbox = $context['sandbox'];
  $cards = &drupal_static(FUNDRAISER_COMMERCE_CARDONFILE_CARDS_STATIC_NAME);

  if (!isset($sandbox['progress'])) {
    // The count of donations visited so far.
    $sandbox['progress'] = 0;
    // Total cards that must be visited. Selecting cards that do not have
    // entries in the {field_data_commerce_cardonfile_profile} table.
    $sandbox['max'] = _fundraiser_commerce_cardonfile_number_of_unupdated_cards();
    // A place to store messages during the run.
    $sandbox['messages'] = array();
    // Last donation read via the query.
    $sandbox['current_card'] = -1;
  }

  // Retrieve the next group of donations to update.
  $existing_card_profiles = db_select('field_data_commerce_cardonfile_profile', 'fdccp')->fields('fdccp', array('entity_id'));
  $result = db_select('commerce_cardonfile', 'cc')
    ->distinct()
    ->fields('cc', array('card_id'))
    ->condition('card_id', $existing_card_profiles, 'NOT IN')
    ->range(0, $limit)
    ->execute()
    ->fetchAll();

  foreach ($result as $row) {
    $sandbox['progress']++;
    $sandbox['current_card'] = $row->card_id;

    $card = commerce_cardonfile_load($row->card_id);
    $card_wrapper = entity_metadata_wrapper('commerce_cardonfile', $card);

    $profile_id = NULL;

    $user_addresses = db_query('SELECT profile_id FROM {commerce_customer_profile} WHERE uid = :uid', array(
      ':uid' => $card->uid,
    ))->fetchAll();
    // If the user has only one address, use that.
    if (count($user_addresses) == 1) {
      $profile_id = $user_addresses[0];
    }
    else {
      $donations = db_query('SELECT did FROM {fundraiser_donation} WHERE card_id = :card_id ORDER BY created DESC', array(
        ':card_id' => $row->card_id,
      ))->execute()->fetchAll();
      // Use the newest donation.
      $did = $donations[0];
      $order = commerce_order_load($did);
      $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
      $profile_id = $order_wrapper->commerce_customer_billing->value();
    }

    $card_wrapper->commerce_cardonfile_profile = $profile_id;
    $card_wrapper->save();
  }

  $context['finished'] = ($sandbox['progress'] >= $sandbox['max']) ? TRUE : ($sandbox['progress'] / $sandbox['max']);

  return $context;
}

/**
 * Donation processing batch 'finished' callback.
 */
function _fundraiser_commerce_cardonfile_processed_cards($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $message = t('@count cards successfully processed.', array('@count' => count($results)));
    $message .= theme('item_list', array('items' => $results));
    drupal_set_message($message);
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE),
    ));
    drupal_set_message($message, 'error');
  }
}

/**
 * Utilify function to retrieve the number of unprocessed donations.
 */
function _fundraiser_commerce_cardonfile_number_of_unupdated_donations() {
  $number_unupdated_donations = &drupal_static(FUNDRAISER_COMMERCE_CARDONFILE_DONATIONS_LEFT_STATIC_NAME);
  if (!$number_unupdated_donations) {
    $query = "SELECT COUNT(DISTINCT did) FROM {fundraiser_donation} WHERE card_id IS NULL AND status = 'payment_received'";
    if (module_exists('fundraiser_sustainers')) {
      $query .= " AND (did IN (SELECT master_did FROM {fundraiser_sustainers} GROUP BY master_did) OR did NOT IN (SELECT did FROM {fundraiser_sustainers}))";
    }

    $number_unupdated_donations = db_query($query)->fetchField();
  }
  return $number_unupdated_donations;
}

/**
 * Helper function to retrieve the number of cards that don't have addresses.
 */
function _fundraiser_commerce_cardonfile_number_of_unupdated_cards() {
  $number_unupdated_cards = &drupal_static(FUNDRAISER_COMMERCE_CARDONFILE_CARDS_LEFT_STATIC_NAME);
  if (!$number_unupdated_cards) {
    $number_unupdated_cards = db_query("SELECT COUNT(DISTINCT card_id) FROM {commerce_cardonfile} WHERE card_id NOT IN (SELECT entity_id FROM {field_data_commerce_cardonfile_profile})")->fetchField();
  }
  return $number_unupdated_cards;
}
