<?php

/**
 * @file
 * Commerce based hook for commerce_litle.
 */

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_litle_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    // Create the payment_method array with the commerce gateway method info name as the key
    'payment_method' => array('commerce_litle_cc' => 'credit', 'commerce_litle_echeck' => 'bank account'),
    // @todo bank account (echeck) recurring is not yet fully implemented.
    'allow_recurring' => array('credit', 'bank account'),
    'allow_refund' => array('credit', 'bank account'),
    'form callback' => 'commerce_litle_fundraiser_commerce_submit_form',
    'cleanup callback' => 'commerce_litle_fundraiser_commerce_cleanup',
    'scrub callback' => 'commerce_litle_fundraiser_commerce_scrub',
    'validate callback' => 'commerce_litle_fundraiser_commerce_validate',
    'expire callback' => 'commerce_litle_fundraiser_commerce_expire',
    'charge callback' => 'commerce_litle_fundraiser_commerce_charge',
    'refund callback' => 'commerce_litle_fundraiser_commerce_refund',
    'update callback' => 'commerce_litle_fundraiser_commerce_update',
    'cardonfile callback' => 'commerce_litle_fundraiser_commerce_cardonfile',
  );
}

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_litle_fundraiser_donation_form_config_check($node) {
  $messages = array();

  foreach ($node->gateways as $method => $gateway) {
    if (!empty($gateway['id'])  && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);

      // Check the credit payment method.
      if ($gateway_config['gateway_details']['method_id'] == 'commerce_litle_cc') {
        // Check for sandbox mode.
        if ($gateway_config['gateway_details']['settings']['commerce_litle_settings_sandbox']) {
          $messages[] = t('The Litle credit card payment method is currently in sandbox mode and will not process live transactions.');
        }
        // Check for a URL configured to use the pre-live environment.
        if (isset($gateway_config['gateway_details']['settings']['commerce_litle_settings_url'])
            && (stripos($gateway_config['gateway_details']['settings']['commerce_litle_settings_url'], 'prelive.') !== FALSE)) {
          $messages[] = t('The Litle credit card payment method is configured to use the pre-live environment and will not process live transactions.');
        }
      }

      // Check the echeck payment method.
      if ($gateway_config['gateway_details']['method_id'] == 'commerce_litle_echeck') {
        // Check for sandbox mode.
        if ($gateway_config['gateway_details']['settings']['commerce_litle_settings_sandbox']) {
          $messages[] = t('The Litle echeck payment method is currently in sandbox mode and will not process live transactions.');
        }
        // Check for a URL configured to use the pre-live environment.
        if (isset($gateway_config['gateway_details']['settings']['commerce_litle_settings_url'])
            && (stripos($gateway_config['gateway_details']['settings']['commerce_litle_settings_url'], 'prelive.') !== FALSE)) {
          $messages[] = t('The Litle echeck payment method is configured to use the pre-live environment and will not process live transactions.');
        }
      }
    }
  }

  return $messages;
}


function commerce_litle_fundraiser_gateway_status_cron_check($details) {
  // Check the credit payment method.
  if ($details['method_id'] == 'commerce_litle_cc') {
    if ($details['settings']['commerce_litle_settings_sandbox']) {
      return 'sandbox';
    }
    else {
      return 'live';
    }
  }
  // Check the echeck payment method.
  if ($details['method_id'] == 'commerce_litle_echeck') {
    if ($details['settings']['commerce_litle_settings_sandbox']) {
      return 'sandbox';
    }
    else {
      return 'live';
    }
  }
}

/**
 * Returns the form fields for this method.
 */
function commerce_litle_fundraiser_commerce_submit_form($payment_method) {
  if ($payment_method == 'credit') {
    module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
    $form = _fundraiser_commerce_credit_card_form();
  }
  elseif ($payment_method == 'bank account') {
    $form = commerce_litle_echeck_submit_form();
    $form = $form['echeck'];
  }
  if (variable_get('commerce_litle_fraud', 0)) {
    $form += commerce_litle_add_fraud_form_elements();
  }
  return $form;
}

/**
 * Convert values to ones that Fundraiser Commerce is looking for.
 */
function commerce_litle_fundraiser_commerce_cleanup($submission_fields) {
  if ($submission_fields['payment_method'] == 'credit') {
    module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
    return _fundraiser_commerce_credit_card_cleanup($submission_fields);
  }
  // Pass through for echeck.
  return $submission_fields;
}

/**
 * Callback function.
 *
 * Return the user entered expiration dates for this submission.
 */
function commerce_litle_fundraiser_commerce_expire($submission_fields, $info = NULL) {

  if ($submission_fields['payment_method'] == 'bank account') {
    if (isset($info['gateway_details']['settings']['commerce_litle_settings_sustainer_series_start_count'])) {
      $exp_month_count = $info['gateway_details']['settings']['commerce_litle_settings_sustainer_series_start_count'];
      $exp_timestamp = strtotime('+' . $exp_month_count . ' months');
    }
    else {
      // pretend expiration date for sustainers
      $exp_timestamp = strtotime('+24 months');
    }

    return array(
      'month' => date('m', $exp_timestamp),
      'year' => date('Y', $exp_timestamp),
    );
  }

  // It's a credit card.
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');

  return _fundraiser_commerce_credit_card_expire($submission_fields);
}

/**
 * Callback function, scrub the data before saving.
 */
function commerce_litle_fundraiser_commerce_scrub($submission_fields, $payment_method) {
  if ($payment_method == 'credit') {
    module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
    return _fundraiser_commerce_credit_card_scrub($submission_fields);
  }
  elseif ($payment_method == 'bank account') {
    // Scrub sensitive donation fields if they exist.
    if (isset($submission_fields['bank account']['accNum'])) {
      $submission_fields['bank account']['accNum'] = substr($submission_fields['bank account']['accNum'], -3);
    }
    if (isset($submission_fields['bank account']['account_number'])) {
      $submission_fields['bank account']['account_number'] = substr($submission_fields['bank account']['account_number'], -3);
    }
    return $submission_fields;
  }
}

/**
 * Validate the submitted values with the commerce validate function.
 */
function commerce_litle_fundraiser_commerce_validate($submission_fields, $payment_fields, $payment_method) {
  // Pass the values to the validation scripts.
  if ($payment_method == 'credit') {
    $form_parents = array_merge($payment_fields['#parents'], array('credit'));
    module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
    _fundraiser_commerce_credit_card_validate($submission_fields, $form_parents);
  }
  elseif ($payment_method == 'bank account') {
    $form_parents = array_merge($payment_fields['#parents'], array('bank account'));
    // Pass the values array with the key the litle module expects.
    $values = array('echeck' => $submission_fields['payment_fields']['bank account']);
    // @todo We need $payment_method (with settings) as the first param here.
    // Passing NULL here means cardonfile will never be used in this case
    // which is not a big deal as long as we don't allow cardonfile selection
    // from the fundraiser forms.
    commerce_litle_echeck_submit_form_validate(NULL, $payment_fields, $values, NULL, $form_parents);
  }

  return $submission_fields;
}

/**
 * Submit the donation values to the Litle charge handler.
 */
function commerce_litle_fundraiser_commerce_charge($method_instance, $donation) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  // Translate from donation settings to the correct values for the plugin.
  $order = commerce_order_load($donation->did);
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $wrapper->commerce_order_total->value();

  // Not actually used anywhere in this system, so ignore it.
  $pane_form = array();
  $pane_values = commerce_litle_pane_values($donation);

  // Execute call to Litle depending on what payment method we have.
  if ($donation->donation['payment_method'] == 'credit') {
    // Add fundraiser commerce data to the pane values array.
    _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation);
    if (variable_get('commerce_litle_fraud', 0) && isset($donation->donation['payment_fields']['credit']['session_id'])) {
      $pane_values['session_id'] = $donation->donation['payment_fields']['credit']['session_id'];
    }
    $method_instance['settings']['cardonfile'] = FALSE;

    if (module_exists('commerce_cardonfile')) {
      // Card on file,
      $method_instance['settings']['cardonfile'] = TRUE;
    }

    $result = commerce_litle_cc_submit_form_submit($method_instance, $pane_form, $pane_values, $order, $charge);
  }
  elseif ($donation->donation['payment_method'] == 'bank account') {
    _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation, 'echeck');
    $method_instance['settings']['cardonfile'] = FALSE;

    if (module_exists('commerce_cardonfile')) {
      // Card on file,
      $method_instance['settings']['cardonfile'] = TRUE;
    }

    $result = commerce_litle_echeck_submit_form_submit($method_instance, $pane_form, $pane_values, $order, $charge);
  }

  // Returns FALSE if the payment failed, returns nothing if successful.
  $success = FALSE;
  if ($result !== FALSE) {
    $success = TRUE;
  }

  // Perform post processing functions.
  _fundraiser_commerce_charge_submit_form_process($success, $method_instance, $pane_values, $donation);

  return $success;
}

/**
 * Get Litle pane values from a donation.
 *
 * Checks which payment type is used and returns the correct values.
 *
 * @param object $donation
 *   The donation to grab the pane values from.
 *
 * @return array
 *   The pane values, keyed by credit_card or echeck.
 */
function commerce_litle_pane_values($donation) {
  $pane_values = array();
  if ($donation->donation['payment_method'] == 'credit') {
    $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);
  }
  elseif ($donation->donation['payment_method'] == 'bank account') {
    $pane_values['echeck'] = $donation->donation['payment_fields']['bank account'];
  }

  return $pane_values;
}

/**
 * Returns the values need to load a card on file profile for Litle.
 *
 * @param object $donation
 *   Fundraiser donation object.
 *
 * @return array
 *   Values ready to be passed into a card on file profile query.
 */
function commerce_litle_cardonfile_fields($donation) {
  $data = array();
  // Use the pane values function to the get the cc/echeck details.
  $pane_values = commerce_litle_pane_values($donation);

  if ($donation->donation['payment_method'] == 'credit') {
    $data = array(
      // Strip the card number down to the last four
      // since thats all card on file will store.
      'card_number' => substr($pane_values['credit_card']['number'], -4),
      'card_exp_month' => $pane_values['credit_card']['exp_month'],
      'card_exp_year' => $pane_values['credit_card']['exp_year'],
    );
  }
  elseif ($donation->donation['payment_method'] == 'bank account') {
    $data = array(
      'card_number' => substr($pane_values['echeck']['accNum'], -3),
    );
  }

  return $data + array('instance_id' => $donation->gateway['id'], 'uid' => $donation->uid);
}

/**
 * Callback function, update card data stored at the gateway and locally.
 *
 * @param object $donation
 *   Fundraiser commerce donation object which should include the
 *   new/updated card data.
 * @param object $card_data
 *   Card on file entity, this should be the current entity for the donation,
 *   new data should be passed in the donation object.
 *
 * @return object|NULL
 *   Card on file entity for the new/updated card data.
 *   NULL if no card data was found.
 */
function commerce_litle_fundraiser_commerce_update($donation, $card_data = NULL) {

  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $payment_method = $donation->gateway['gateway_details'];

  // If no card data was passed load the profile from the donation if it's set.
  if (empty($card_data) && !empty($donation->data['cardonfile'])) {
    $card_data = commerce_cardonfile_load($donation->data['cardonfile']);
  }

  // Pull the new credit card values from the donation.
  $pane_values = commerce_litle_pane_values($donation);

  if (empty($pane_values['credit_card']) && (empty($pane_values['echeck']['routingNum']) || empty($pane_values['echeck']['accNum']))) {
    return $card_data;
  }

  // Load the card on file values for this donation.
  $cardonfile_fields = commerce_litle_cardonfile_fields($donation);

  // Check the card number to see if it matches the current card data number.
  // If it does then we use the update function.
  // if its a new card check if we should create a new vault profile.
  $action = 'update';
  if (empty($card_data)) {
    $action = 'new';
  }
  elseif ($cardonfile_fields['card_number'] != $card_data->card_number) {
    // This is a new card for this donation,
    // make sure we don't have an existing profile.
    // First check with the card number and date.
    if ($existing = _fundraiser_commerce_cardonfile_match($cardonfile_fields)) {
      // Set the existing card on the donation and return it.
      $donation->data['cardonfile'] = $existing->card_id;
      $card_data = $existing;
    }
    // Check if we have a mismatched exp date record.
    // This only applies to credit cards.
    elseif ($donation->donation['payment_method'] == 'credit' && ($mismatch = _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields))) {
      $donation->data['cardonfile'] = $mismatch->card_id;
      $card_data = $mismatch;
    }
    // Nothing stored locally so create a new profile.
    else {
      $action = 'new';
    }
  }

  if ($action == 'update') {
    $donation->old_card = clone $card_data;
    // This is last four digits for credit cards and last three for echecks.
    $card_data->card_number = $cardonfile_fields['card_number'];

    // Only credit cards have an expiration date.
    if ($donation->donation['payment_method'] == 'credit') {
      $card_data->card_exp_month = $cardonfile_fields['card_exp_month'];
      $card_data->card_exp_year = $cardonfile_fields['card_exp_year'];

      // This is the full number.
      $account_number = $pane_values['credit_card']['number'];
      $card_validation_number = $pane_values['credit_card']['code'];
      $response = commerce_litle_cc_vault_cardonfile_register_token($payment_method, $account_number, $card_validation_number, $donation->did);

      $token = commerce_litle_get_token_from_response($response);
      if (!empty($token)) {
        $card_data->remote_id = $token;
        commerce_cardonfile_save($card_data);
        return $card_data;
      }
    }
    elseif ($donation->donation['payment_method'] == 'bank account') {
      $card_data->card_exp_month = '';
      $card_data->card_exp_year = '';

      $routing_number = $pane_values['echeck']['routingNum'];
      $account_number = $pane_values['echeck']['accNum'];
      $response = commerce_litle_eheck_vault_cardonfile_register_token($payment_method, $routing_number, $account_number, $donation->did);

      $token = commerce_litle_get_token_from_response($response);
      if (!empty($token)) {
        $card_data->remote_id = commerce_litle_format_cardonfile_remote_id($pane_values, $token);
        commerce_cardonfile_save($card_data);
        return $card_data;
      }
    }
  }
  // Create a new card on file record.
  elseif ($action == 'new') {
    $card_name = '';
    $order = commerce_order_load($donation->did);
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    // Card name is used to save the card name with the other card data..
    if ($order_wrapper->commerce_customer_billing->value()) {
      $billing_address = $order_wrapper->commerce_customer_billing->commerce_customer_address->value();
      $card_name = $billing_address['name_line'];
    }

    if ($donation->donation['payment_method'] == 'credit') {

      $account_number = $pane_values['credit_card']['number'];
      $card_validation_number = $pane_values['credit_card']['code'];
      $response = commerce_litle_cc_vault_cardonfile_register_token($payment_method, $account_number, $card_validation_number, $donation->did);

      $token = commerce_litle_get_token_from_response($response);

      // If the Vault token creation was a success,
      // store the new card on file data locally.
      if (!empty($token)) {

        // Get the card type.
        $card_type = commerce_payment_validate_credit_card_type($pane_values['credit_card']['number'], array_keys(commerce_payment_credit_card_types()));
        $all_types = commerce_payment_credit_card_types();
        $card_type = !empty($all_types[$card_type]) ? $all_types[$card_type] : 'card';

        // Store the token with cardonfile.
        $card_data = commerce_cardonfile_new();
        $card_data->uid = $order->uid;
        $card_data->payment_method = $payment_method['method_id'];
        $card_data->instance_id = $payment_method['instance_id'];
        $card_data->remote_id = $token;
        $card_data->card_type = !empty($card_type) ? $card_type : 'card';
        $card_data->card_name = $card_name;
        $card_data->card_number = substr($account_number, -4);
        $card_data->card_exp_month = $pane_values['credit_card']['exp_month'];
        $card_data->card_exp_year = $pane_values['credit_card']['exp_year'];
        $card_data->status = 1;

        // Save and log the creation of the new card on file.
        commerce_cardonfile_save($card_data);
        watchdog('commerce_litle', 'Card on file registered and saved to user @uid.', array('@uid' => $order->uid));

        // Set the value on the donation.
        $donation->data['cardonfile'] = $card_data->card_id;
        return $card_data;
      }
    }
    elseif ($donation->donation['payment_method'] == 'bank account') {
      $routing_number = $pane_values['echeck']['routingNum'];
      $account_number = $pane_values['echeck']['accNum'];
      $response = commerce_litle_eheck_vault_cardonfile_register_token($payment_method, $routing_number, $account_number, $donation->did);

      $token = commerce_litle_get_token_from_response($response);

      // If the Vault token creation was a success,
      // store the new card on file data locally.
      if (!empty($token)) {

        // For echeck, card type is the account type.
        $card_type = $pane_values['echeck']['accType'];
        $remote_id = commerce_litle_format_cardonfile_remote_id($pane_values, $token);

        // Store the token with cardonfile.
        $card_data = commerce_cardonfile_new();
        $card_data->uid = $order->uid;
        $card_data->payment_method = $payment_method['method_id'];
        $card_data->instance_id = $payment_method['instance_id'];
        $card_data->remote_id = $remote_id;
        $card_data->card_type = $card_type;
        $card_data->card_name = $card_name;
        $card_data->card_number = substr($account_number, -3);
        $card_data->card_exp_month = '';
        $card_data->card_exp_year = '';
        $card_data->status = 1;

        // Save and log the creation of the new card on file.
        commerce_cardonfile_save($card_data);
        watchdog('commerce_litle', 'Card on file registered and saved to user @uid.', array('@uid' => $order->uid));

        // Set the value on the donation.
        $donation->data['cardonfile'] = $card_data->card_id;
        return $card_data;
      }
    }
  }
}

/**
 * Card on file callback.
 *
 * We use a custom callback since we may need to update an existing vault
 * profile.
 */
function commerce_litle_fundraiser_commerce_cardonfile($donation) {
  // Use standard function to setup the card on file fields.
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $cardonfile_fields = commerce_litle_cardonfile_fields($donation);

  // So we need to search locally to see if we have an existing profile for
  // a card number.
  // Only perform this when we're in an active non-reference charge.
  if (isset($donation->reference_charge) && $donation->reference_charge === FALSE) {
    if ($donation->donation['payment_method'] == 'credit') {
      // Query the db for an existing record.
      $card_data = _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields);
    }
    else {
      // Echeck records never get updated.
      $card_data = FALSE;
    }

    // If we have a profile for this user update it locally.
    if ($card_data) {
      // Make a card update request to Litle.
      commerce_litle_fundraiser_commerce_update($donation, $card_data);
    }
  }

  return $cardonfile_fields;
}

/**
 * Refund callback for Litle.
 *
 * Wraps the admin form used for refund with our own system.
 */
function commerce_litle_fundraiser_commerce_refund($payment_method, $refund) {
  // Retrieve data.
  $refunding_order = commerce_order_load($refund->did);
  $transactions = commerce_payment_transaction_load_multiple(array(), $conditions = array('order_id' => $refunding_order->order_id));
  // The first one is the original donation.
  $transaction = array_shift($transactions);

  // Set up the values needed for the transaction to go through.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);
  // Submit the donation form.
  module_load_include('inc', 'commerce_litle', 'commerce_litle.admin');
  return commerce_litle_credit_request($payment_method, $transaction, $refunding_order, $refund->amount);
}

/**
 * Implements hook_commerce_litle_request_alter().
 *
 * Change the orderSource to recurring if this is a recurring
 * (but not original) sustainer charge via a credit card.
 */
function commerce_litle_commerce_litle_request_alter(&$request, $context) {
  if ($context['type'] == 'cc_vault') {
    $request['orderSource'] = 'recurring';
  }
}
