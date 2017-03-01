<?php

/**
 * @file
 * Commerce based hook for commerce_payment_example.
 */

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_payment_example_fundraiser_donation_form_config_check($node) {
  foreach ($node->gateways as $method => $gateway) {
    if (!empty($gateway['id']) && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);
      if ($gateway_config['gateway_details']['method_id'] == 'commerce_payment_example') {
        return array(t('This form is using the example credit payment gateway and will not process live transactions.'));
      }
    }
  }
}

function commerce_payment_example_fundraiser_gateway_status_cron_check($details) {
  if ($details['method_id'] == 'commerce_payment_example') {
    return 'example gateway';
  }
}

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_payment_example_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    'payment_method' => array('credit'),
    'allow_recurring' => array('credit'),
    'offsite_recurring' => array('credit'),
    'allow_refund' => array('credit'),
    'form callback' => 'commerce_payment_example_fundraiser_commerce_form',
    'cleanup callback' => 'commerce_payment_example_fundraiser_commerce_cleanup',
    'scrub callback' => 'commerce_payment_example_fundraiser_commerce_scrub',
    'expire callback' => 'commerce_payment_example_fundraiser_commerce_expire',
    'charge callback' => 'commerce_payment_example_fundraiser_commerce_charge',
    'refund callback' => 'commerce_payment_example_fundraiser_commerce_refund',
    'update callback' => 'commerce_payment_example_fundraiser_commerce_update',
    'cardonfile callback' => 'commerce_payment_example_fundraiser_commerce_cardonfile',
    'cardonfile' => array(
      'charge callback' => 'commerce_payment_example_fundraiser_commerce_cardonfile_charge',
      'update callback' => 'commerce_payment_example_fundraiser_commerce_cardonfile_update',
      'delete callback' => 'commerce_payment_example_fundraiser_commerce_cardonfile_delete',
    ),
  );
}

/**
 * Callback function, use this form for the given gateway.
 */
function commerce_payment_example_fundraiser_commerce_form($payment_method) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_form();
}

/**
 * Callback function, clean up the submission results after submission before they are returned to fundraiser.
 */
function commerce_payment_example_fundraiser_commerce_cleanup($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_cleanup($submission_fields);
}

/**
 * Callback function, scrub the fields before they get saved.
 */
function commerce_payment_example_fundraiser_commerce_scrub($submission_fields, $payment_method) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_scrub($submission_fields);
}

/**
 * Callback function, return the user entered experation dates for this submission.
 */
function commerce_payment_example_fundraiser_commerce_expire($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_expire($submission_fields);
}

/**
 * Callback function, charge the gateway.
 */
function commerce_payment_example_fundraiser_commerce_charge($method_instance, $donation) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');

  // Translate donation settings to the correct values for the commerce gateway.
  $order = commerce_order_load($donation->did);
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $order_wrapper->commerce_order_total->value();
  $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);

  // Add fundraiser commerce data to the pane values array.
  _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation);

  // Process the transaction. First look for special values that force a
  // failure.
  if (commerce_payment_example_is_simulated_failure($donation)) {
    // Add a payment transaction record for failed payments.
    $transaction = commerce_payment_transaction_new('commerce_payment_example', $order->order_id);
    $transaction->instance_id = $method_instance['instance_id'];
    $transaction->amount = $charge['amount'];
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
    $transaction->remote_status = 'remote_failed';
    $transaction->message = t('Failed due to sustainer failure simulation mode.');
    commerce_payment_transaction_save($transaction);
    return FALSE;
  }
  // Pass the values through to the submit function.
  else {
    // During a reference charge the example gateway expects a credit card of a
    // certain length for the transaction message.
    if (!empty($donation->reference_charge)) {
      $pane_values['credit_card']['number']  = str_pad($pane_values['credit_card']['number'], 8, 'x', STR_PAD_LEFT);
    }

    commerce_payment_example_fundraiser_commerce_update($donation, NULL, 'new');

    $number = $pane_values['credit_card']['number'];
    $pane_values['credit_card']['number'] = substr($number, 0, 4) . str_repeat('-', strlen($number) - 8) . substr($number, -4);

    $order->data['commerce_payment_example'] = $pane_values;

    // Every attempted transaction should result in a new transaction entity
    // being created for the order to log either the success or the failure.
    commerce_payment_example_transaction($method_instance, $order, $charge);

    return TRUE;
  }
}


/**
 * Determines whether the gateway should simulate a payment failure.
 *
 * Failure is simulated by either a designed card number or postal code.
 *
 * @param $donation
 *   The donation being processed.
 */
function commerce_payment_example_is_simulated_failure($donation) {
  if (!$donation->reference_charge && $donation->donation['payment_fields']['credit']['card_number'] == '4222222222222') {
    return TRUE;
  }

  if ($donation->reference_charge && $donation->donation['zip'] == '55555') {
    return TRUE;
  }

  return FALSE;
}


/**
 * Callback function, refund the gateway.
 */
function commerce_payment_example_fundraiser_commerce_refund($payment_method, $refund) {
  // Retrieve data.
  $order = commerce_order_load($refund->did);
  $transactions = commerce_payment_transaction_load_multiple(array(), $conditions = array('order_id' => $order->order_id));
  $transaction = array_shift($transactions); // The first one is the original donation.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);
  // Create a new transaction to record the credit.
  $credit_transaction = commerce_payment_transaction_new('commerce_payment_example', $order->order_id);
  $credit_transaction->instance_id = $payment_method['instance_id'];
  $credit_transaction->remote_id = 'remote_id';
  $credit_amount = commerce_currency_decimal_to_amount($refund->amount, $transaction->currency_code);
  $credit_transaction->amount = $credit_amount * -1;
  $credit_transaction->currency_code = $transaction->currency_code;
  $credit_transaction->payload[REQUEST_TIME] = 'success';
  $credit_transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
  $credit_transaction->remote_status = 'remote_success';
  $credit_transaction->message = t('Credited back.');
  // Save the credit transaction.
  commerce_payment_transaction_save($credit_transaction);
  return TRUE;
}

/**
 * Callback function, update card data stored.
 *
 * @param object $donation
 *   Fundraiser commerce donation object which should include the new/updated
 *   card data.
 * @param object $card_data
 *   Card on file entity, this should be the current entity for the donation,
 *   new data should be passed in the donation object.
 *
 * @return object
 *   Card on file entity for the new/updated card data.
 */
function commerce_payment_example_fundraiser_commerce_update($donation, $card_data = NULL, $action = 'update') {
  dpm(__FUNCTION__);
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $payment_method = $donation->gateway['gateway_details'];

  // If no card data was passed load the profile from the donation if its set.
  if ($action == 'update') {
    if (empty($card_data) && !empty($donation->data['cardonfile'])) {
      $card_data = commerce_cardonfile_load($donation->data['cardonfile']);
    }

    if (empty($card_data)) {
      // Log the missing card data.
      watchdog(
        'fundraiser',
        'A card update was attempted on donation @id but no stored card data was found.',
        array('@id' => $donation->did),
        WATCHDOG_CRITICAL
      );

      return $card_data;
    }
  }

  // Pull the new credit card values from the donation.
  $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);

  // Load the card on file values for this donation.
  $cardonfile_fields = _fundraiser_commerce_credit_card_cardonfile_fields($donation);

  // If the card_number in $cardonfile_fields is FALSE, the number wasn't
  // entered, so the address might need updating.
  if ($action == 'update' && !empty($cardonfile_fields['card_number']) && $cardonfile_fields['card_number'] != $card_data->card_number) {
    // This is a new card for this donation, make sure we don't have an existing
    // profile. First check with the card number and date.
    if ($existing = _fundraiser_commerce_cardonfile_match($cardonfile_fields)) {
      // Set the existing card on the donation and return it.
      $donation->data['cardonfile'] = $existing->card_id;
      $card_data = $existing;
      return $card_data;
    }
    // Check if we have a mismatched exp date record.
    elseif ($mismatch = _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields)) {
      $donation->data['cardonfile'] = $mismatch->card_id;
      $card_data = $mismatch;
    }
    // Nothing stored locally so create a new profile.
    else {
      $action = 'new';
    }
  }

  if ($action == 'update') {
    if (!empty($cardonfile_fields['card_number'])) {
      $card_data->card_number = $cardonfile_fields['card_number'];
    }
    $card_data->card_exp_month = $cardonfile_fields['card_exp_month'];
    $card_data->card_exp_year = $cardonfile_fields['card_exp_year'];

    // We're mirroring the commerce_authnet_cim_cardonfile_update function so
    // setup values that function uses, see that function for details.
    $form['credit_card']['number']['#default_value'] = '';
    $form_state['values']['credit_card']['number'] = $pane_values['credit_card']['number'];
    $form_state['values']['credit_card']['owner'] = isset($donation->donation['first_name']) ? $donation->donation['first_name'] : '';
    $form_state['values']['credit_card']['owner'] .= isset($donation->donation['last_name']) ? ' ' . $donation->donation['last_name'] : '';
    $form_state['values']['credit_card']['address'] = isset($donation->donation['address']) ? $donation->donation['address'] : '';
    $form_state['values']['credit_card']['address'] .= isset($donation->donation['address_line_2']) ? ' ' . $donation->donation['address_line_2'] : '';
    $form_state['values']['credit_card']['city'] = isset($donation->donation['city']) ? $donation->donation['city'] : '';
    $form_state['values']['credit_card']['state'] = isset($donation->donation['state']) ? $donation->donation['state'] : '';
    $form_state['values']['credit_card']['zip'] = isset($donation->donation['zip']) ? $donation->donation['zip'] : '';
    $form_state['values']['credit_card']['country'] = isset($donation->donation['country']) ? $donation->donation['country'] : '';

    commerce_cardonfile_save($card_data);
    return $card_data;
  }
  // Create a new card on file record.
  elseif ($action == 'new') {
    // We're using the built in function that requires a payment details array
    // and the order.
    $payment_details = array(
      'cardNumber' => $pane_values['credit_card']['number'],
      'expirationDate' => $pane_values['credit_card']['exp_year'] . '-' . sprintf("%02d", $pane_values['credit_card']['exp_month']),
    );

    if (isset($pane_values['credit_card']['code'])) {
      $payment_details['cardCode'] = $pane_values['credit_card']['code'];
    }

    // Load the order.
    $order = commerce_order_load($donation->did);

    // Build a remote ID that includes the Customer Profile ID and the
    // Payment Profile ID.
    $remote_id = (string) $payment_method['method_id'] . '|' . time();

    // Get the card type.
    $card_type = commerce_payment_validate_credit_card_type($pane_values['credit_card']['number'], array_keys(commerce_payment_credit_card_types()));
    $all_types = commerce_payment_credit_card_types();
    $card_type = !empty($all_types[$card_type]) ? $all_types[$card_type] : 'card';

    // Create a new cardonfile entity.
    $new_data = commerce_cardonfile_new();
    $new_data->uid = $order->uid;
    $new_data->payment_method = $payment_method['method_id'];
    $new_data->instance_id = $payment_method['instance_id'];
    $new_data->remote_id = $remote_id;
    $new_data->card_type = $card_type;
    $new_data->card_name = !empty($card_data->card_name) ? $card_data->card_name : '';
    $new_data->card_number = $cardonfile_fields['card_number'];
    $new_data->card_exp_month = $cardonfile_fields['card_exp_month'];
    $new_data->card_exp_year = $cardonfile_fields['card_exp_year'];
    $new_data->status = 1;

    // Save and log the creation of the new card on file..
    commerce_cardonfile_save($new_data);
    // Set the value on the donation.
    $donation->data['cardonfile'] = $new_data->card_id;
    return $new_data;
  }

  return $card_data;
}

/**
 * Cardonfile callback.
 */
function commerce_payment_example_fundraiser_commerce_cardonfile($donation) {
  // Use standard function to setup the card on file fields.
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $cardonfile_fields = _fundraiser_commerce_credit_card_cardonfile_fields($donation);

  if (isset($donation->reference_charge) && $donation->reference_charge === FALSE) {
    // Query the db for an existing record.
    $card_data = _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields);
    if ($card_data) {
      commerce_payment_example_fundraiser_commerce_update($donation, $card_data);
    }
  }

  return $cardonfile_fields;
}

function commerce_payment_example_fundraiser_commerce_cardonfile_charge($payment_method, $card_data, $order, $charge = NULL) {
  // Format order total for transaction.
  if (!isset($charge)) {
    $wrapper = entity_metadata_wrapper('commerce_order', $order);
    $charge = commerce_line_items_total($wrapper->commerce_line_items);
  }

  // @todo Set up way to return variable results based on the payment info
  // entered, like intentionally failing certain values are input.
  $response = array(
    // The response code.
    0 => 1,
    // The remote status.
    11 => 'success',
    // The error message.
    3 => "This shouldn't appear yet.",
  );

  // Prepare a transaction object to represent the transaction attempt.
  $transaction = commerce_payment_transaction_new('authnet_echeck', $order->order_id);
  $transaction->instance_id = $payment_method['instance_id'];
  $transaction->remote_id = 'commerce_payment_example|' . time();
  $transaction->amount = $charge['amount'];
  $transaction->currency_code = $charge['currency_code'];
  $transaction->payload[REQUEST_TIME] = $response;

  // If we didn't get an approval response code...
  if ($response[0] != '1') {
    // Create a failed transaction with the error message.
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
  }
  else {
    $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
  }

  // Store the type of transaction in the remote status.
  $transaction->remote_status = $response[11];

  // Build a meaningful response message.
  $message = array(
    '<b>' . commerce_authnet_reverse_txn_type(commerce_authnet_txn_type($payment_method['settings']['txn_type'])) . '</b>',
    '<b>' . ($response[0] != '1' ? t('REJECTED') : t('ACCEPTED')) . ':</b> ' . check_plain($response[3]),
    t('AVS response: @avs', array('@avs' => commerce_authnet_avs_response($response[5]))),
  );

  $transaction->message = implode('<br />', $message);

  // Save the transaction information.
  commerce_payment_transaction_save($transaction);

  // If the payment failed, display an error and rebuild the form.
  if ($response[0] != '1') {
    drupal_set_message(t('We received the following error processing your card. Please enter your information again or try a different card.'), 'error');
    drupal_set_message(check_plain($response[3]), 'error');
    return FALSE;
  }

  return TRUE;
}

function commerce_payment_example_fundraiser_commerce_cardonfile_update($form, &$form_state, $payment_method, $card_data) {
  return TRUE;
}

function commerce_payment_example_fundraiser_commerce_cardonfile_delete() {
  return TRUE;
}
