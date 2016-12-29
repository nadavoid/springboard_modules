<?php

/**
 * @file
 * Support for Fundraiser donations
 */

/**
 * Destination class for a fundraiser donation
 */
class FundraiserDonationDestination extends MigrateDestination {
  public function __construct() {
    parent::__construct();
  }

  static public function getKeySchema() {
    return array(
      'did' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'ID of the donation',
      ),
    );
  }

  public function __toString() {
    $output = t('Donations');
    return $output;
  }

  /**
   * Delete a batch of donations at once.
   *
   * @param $dids
   *  Array of donation IDs to be deleted.
   */
  public function bulkRollback(array $dids) {
    $rids = array();

    foreach ($dids as $did) {
      $donation = fundraiser_donation_get_donation($did);

      // Allow add-on modules to do their cleanup
      fundraiser_donation_delete($donation);

      // Delete the webform submission
      if (!empty($donation->sid)) {
        module_load_include('inc', 'webform', 'includes/webform.submissions');
        $node = node_load($donation->nid);
        $submission = webform_get_submission($node->nid, $donation->sid);
        webform_submission_delete($node, $submission);
      }

      $recurring = _fundraiser_sustainers_get_donations_recurr_by_masterdid($donation->did);

      if (!empty($recurring)) {
        foreach ($recurring as $recur) {
          // Delete the main record
          db_delete('fundraiser_donation')
            ->condition('did', $recur->did)
            ->execute();

          $rids[] = $recur->did;

          // Delete sustainer log records.
          db_delete('fundraiser_sustainers_log')->condition('did', $recur->did)->execute();
        }

        commerce_order_delete_multiple($rids);
      }

      // Delete cardonfile record.
      if (!empty($donation->data['cardonfile'])) {
        commerce_cardonfile_delete($donation->data['cardonfile']);
      }

      // Delete the main record
      db_delete('fundraiser_donation')
        ->condition('did', $did)
        ->execute();
      
      // Fundraiser sustainers implemenation of hook_fundraiser_donation_delete() 
      // doesn't remove associated fundraiser_sustainer records :(
      db_delete('fundraiser_sustainers')->condition('master_did', $did)->execute();
      entity_delete('fundraiser_sustainers_series', $did);

      // Delete sustainer log records.
      db_delete('fundraiser_sustainers_log')->condition('did', $did)->execute();
    }

    // Delete the orders
    commerce_order_delete_multiple($dids);

    // Delete everything from the queue
    db_delete('salesforce_queue')
      ->condition('drupal_id', array_merge($rids, $dids), 'IN')
      ->condition('module', array('salesforce_donation', 'fundraiser_sustainers'), 'IN')
      ->execute();

    $this->completeRollback($dids); 
  }

  /**
   * Import a single row.
   *
   * @param $entity
   *  Object object to build. Prefilled with any fields mapped in the Migration.
   * @param $row
   *  Raw source data object - passed through to prepare/complete handlers.
   * @return array
   *  Array of key fields of the object that was saved if
   *  successful. FALSE on failure.
   */
  public function import(stdClass $entity, stdClass $row) {
    $donation = $this->buildDonationObject($entity);

    $migration = Migration::currentMigration();
    // Updating previously-migrated donation?
    if (isset($row->migrate_map_destid1)) {
      if (isset($donation->did)) {
        if ($donation->did != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming did !did and map destination did !destid1 don't match",
            array('!did' => $donation->did, '!destid1' => $row->migrate_map_destid1)));
        }
      }
      else {
        $donation->did = $row->migrate_map_destid1;
      }
    }

    if (isset($donation->did)) {
      $current_donation = fundraiser_donation_get_donation($donation->did);
    }

    if (!empty($current_donation)) {
      $updating = TRUE;
      // We'll need the recurring items for later.
      $donation->recurring = $current_donation->recurring;
    }
    else {
      $updating = FALSE;
    }

    // Create a webform submission if that has been passed
    if (!empty($row->submission)) {
      module_load_include('inc', 'webform', 'includes/webform.submissions');

      $node = node_load($donation->nid);
      $submission = new stdClass();
      $submission->data = array();
      $submission->uid = $donation->uid;
      $submission->submitted = time();
      foreach ($row->submission as $field_name => $value) {
        $field = _fundraiser_webform_get_component_by_nid_key($node->nid, $field_name);
        if (!empty($field)) {
          $submission->data[$field->cid] = array('value' => array($value));
        }
      }

      // If updating and there is a current submission, update that record.
      if ($updating && !empty($current_donation->sid)) {
        $submission->sid = $current_donation->sid;
        webform_submission_update($node, $submission);
      }
      // Create a new submission.
      else {
        $donation->sid = webform_submission_insert($node, $submission);
      }
 
    }

    // Create a card on file
    if (!empty($row->cardonfile)) {
      // If updating and there is a current card on file record, update that record.
      if ($updating && !empty($current_donation->data['cardonfile'])) {
        $card_data = commerce_cardonfile_load($current_donation->data['cardonfile']);
      }
      else {
        $card_data = commerce_cardonfile_new();
      }
 
      $card_data->uid = $donation->uid;
      $card_data->payment_method = $donation->gateway['gateway_details']['method_id'];
      $card_data->instance_id = $donation->gateway['gateway_details']['instance_id'];

      // Add the passed in values
      foreach ($row->cardonfile as $key => $value) {
        $card_data->{$key} = $value;
      }

      // Save the card on file record.
      commerce_cardonfile_save($card_data);
      if (!empty($card_data->card_id)) {
        $donation->data['cardonfile'] = $card_data->card_id;
      }
    }

    if ($updating) {
      fundraiser_donation_update($donation);
      $this->numUpdated++;
    }
    else {
      fundraiser_donation_create($donation);
      $this->numCreated++;
    }

    if (!empty($donation->did)) {
      $this->complete($donation, $row);
      return array($donation->did);
    }
    else {
      return FALSE;
    }
  }

  /**
   * Returns a list of fields available to be mapped.
   *
   * @param Migration $migration
   *  Optionally, the migration containing this destination.
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields($migration = NULL) {
    $values = array(
      'created',
      'changed',
      'uid',
      'mail',
      'sid',
      'status',
      'nid',
      'gateway',
      'country',
      'first_name',
      'last_name',
      'state',
      'address',
      'address_line_2',
      'city',
      'zip',
      'payment_method',
      'payment_fields',
      'cardonfile',
      'amount',
      'currency',
      'recurs_monthly',
      'data',
    );

    foreach ($values as $value) {
      $fields[$value] = $value;
    }

    return $fields;
  }

  public function buildDonationObject($values) {
    $migration = Migration::currentMigration();

    $donation = new stdClass();
    $donation->created = $values->created;
    $donation->changed = REQUEST_TIME;

    // Load up the current user if there is one
    if ($user = user_load_by_mail($values->mail)) {
      $donation->user = $user;
      $donation->uid = $user->uid;
    }
    else {
      // Set the user as anonymous, will be created by fundraiser later
      $donation->user = new stdClass();
      $donation->user->uid = 0;
      $donation->uid = 0;
    }

    // For our purposes we'll store the original order id as the sid
    $donation->sid = '';

    // This payment has been captured
    $donation->status = 'payment_received';

    // Set the node
    if (!empty($values->nid)) {
      $node = node_load($values->nid);
      if (!empty($node)) {
        $donation->node = $node;
        $donation->nid = $node->nid;
      }
      else {
        $migration->saveMessage(t('A node with nid %nid ' . ' was not found on the site.',
          array('%nid' => $values->nid)));
      }
    }

    $donation->gateway = _fundraiser_gateway_info($values->gateway);

    $donation->donation = array();

    // Populate the necessary donation values
    $donation->donation['mail'] = $values->mail;
    $donation->donation['country'] = $values->country;
    $donation->donation['first_name'] = $values->first_name;
    $donation->donation['last_name'] = $values->last_name;
    $donation->donation['state'] = $values->state;
    $donation->donation['address'] = $values->address;
    $donation->donation['address_line_2'] = $values->address_line_2;
    $donation->donation['city'] = $values->city;
    $donation->donation['zip'] = $values->zip;
    $donation->donation['payment_method'] = $values->payment_method;
    $donation->donation['payment_fields'] = $values->payment_fields;

    // Set the amount & currency
    $donation->donation['amount'] = $values->amount;
    $donation->donation['currency'] = $values->currency;

    // Flag the donation recurring or not
    $donation->donation['recurs_monthly'] = !empty($values->recurs_monthly) ? $values->recurs_monthly : FALSE;

    $donation->data = $values->data;

    return $donation;
  }

  public function complete($donation, $row) {
    // Fire off all the success hooks
    if (!empty($row->donation_success)) {
      fundraiser_donation_success($donation);
    }

    // Handle any custom reccuring
    if (!empty($row->create_sustainer_series)) {
      $this->createRecurringSeries($donation, $row);
    }

    // Send to the queue
    if (!empty($row->send_to_queue)) {
      salesforce_donation_fundraiser_donation_success($donation);
    }

    $migration = Migration::currentMigration();

    // Call any complete handler for this specific Migration.
    if (method_exists($migration, 'complete')) {
      $migration->complete($donation, $row);
    }
  }

  public function completeRollback($entity_ids) {
    $migration = Migration::currentMigration();

    // Call any complete handler for this specific Migration.
    if (method_exists($migration, 'completeRollback')) {
      $migration->completeRollback($entity_ids);
    }
  }

  public function createRecurringSeries($donation, $row) {
    $info = $donation->gateway;
    $migration = Migration::currentMigration();

    // Ensure the payment method supports recurring.
    if (_fundraiser_sustainers_supports_recurring($info['allow_recurring'], $donation->donation['payment_method'])) { // Allows recurring

      // Create a recurring object if we aren't updating.
      if (empty($donation->recurring)) {
        $donation->recurring = new stdClass();
        $donation->recurring->master_did = $donation->did;
      }
 
      // Add/update the master to the recurring table.
      $sustainer_key = fundraiser_sustainers_get_sustainer_key_value();
      $recurring_donation = array(
        'did' => $donation->did,
        'master_did' => $donation->did,
        'next_charge' => $row->recurring_start_date < time() ? time() : $row->recurring_start_date,
        'sustainer_key' => $sustainer_key,
      );

      // if this master donation should be flagged as success
      if (!empty($row->donation_success)) {
        $recurring_donation['gateway_resp'] = 'success';
        $recurring_donation['attempts'] = 1;
      }

      // Update the recurring table.
      _fundraiser_sustainers_create_recurring($recurring_donation);

      // Create the sustainer series entity      
      $fundraiser_sustainers_series_data = array(
        'did' => $donation->did,
        'uid' => $donation->uid,
        'amount' => $donation->donation['amount'],
        'installments' => _fundraiser_sustainers_calculate_installments($donation)+1, // +1 because the function assumes the start date is now!
      );

      db_insert('fundraiser_sustainers_series')
        ->fields($fundraiser_sustainers_series_data)
        ->execute();

      // Now handle the recurring object, and the rest of the orders.
      if (!_fundraiser_sustainers_offsite_recurring($info['offsite_recurring'], $donation->donation['payment_method'])) { // Is not an offline recurring
        // Load the newly created recurring donation into salesforce.
        if (module_exists('salesforce_genmap')) {
          $sf_donation = fundraiser_donation_get_donation($donation->did, TRUE);
          salesforce_genmap_send_object_to_queue('fundraiser_sustainers', 'insert', $sf_donation->node, $sf_donation->did, $sf_donation, 'recurring_donation');
        }

        if (isset($info['expire callback'])) {
          $expiration_func = $info['expire callback'];
          if (function_exists($expiration_func)) {
            $expires = $expiration_func($donation->donation, $info);

            if (empty($expires['month']) || empty($expires['year'])) {
              $migration->saveMessage('No expiration date found for this donation.', MigrationBase::MESSAGE_ERROR);
              return;
            }

            // Create the series
            $this->createFutureMonthlyOrders($donation, $expires['month'], $expires['year'], $row->recurring_start_date);
          }
        }
      }
    }
  }

  public function createFutureMonthlyOrders($donation, $month, $year, $start) {
    _fundraiser_sustainers_create_future_orders($donation, $month, $year, NULL, $start);
  }
}
