<?php

/**
 * @file
 * Commerce based hook for commerce_cybersource.
 */

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_cybersource_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    'payment_method' => array('credit'),
    'allow_recurring' => array('credit'),
    'allow_refund' => array('credit'),
    'form callback' => 'commerce_cybersource_fundraiser_commerce_form',
    'cleanup callback' => 'commerce_cybersource_fundraiser_commerce_cleanup',
    'scrub callback' => 'commerce_cybersource_fundraiser_commerce_scrub',
    'expire callback' => 'commerce_cybersource_fundraiser_commerce_expire',
    'validate callback' => 'commerce_cybersource_fundraiser_commerce_validate',
    'charge callback' => 'commerce_cybersource_fundraiser_commerce_charge',
    'update callback' => 'commerce_cybersource_fundraiser_commerce_update',
    'refund callback' => 'commerce_cybersource_fundraiser_commerce_refund',
    'cardonfile callback' => 'commerce_cybersource_fundraiser_commerce_cardonfile',
  );
}

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_cybersource_fundraiser_donation_form_config_check($node) {
  $messages = array();

  foreach ($node->gateways as $method => $gateway) {
    if (!empty($gateway['id'])  && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);
      // Check the credit payment method.
      if ($gateway_config['gateway_details']['method_id'] == 'cybersource_soap_cc' &&
        $gateway_config['gateway_details']['settings']['transaction_mode'] != 'production') {
        $messages[] = t('The Cybersource payment method is currently in test mode and will not process live transactions.');
      }
    }
  }

  return $messages;
}


/**
 * Callback function, use this form for the given gateway.
 */
function commerce_cybersource_fundraiser_commerce_form($payment_method) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_form();
}

/**
 * Callback function, clean up the submission results after submission before they are returned to fundraiser.
 */
function commerce_cybersource_fundraiser_commerce_cleanup($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_cleanup($submission_fields);
}

/**
 * Callback function, scrub data.
 */
function commerce_cybersource_fundraiser_commerce_scrub($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_scrub($submission_fields);
}

/**
 * Callback function, return the user entered experation dates for this submission.
 */
function commerce_cybersource_fundraiser_commerce_expire($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_expire($submission_fields);
}

/**
 * Validate the submitted values with the commerce validate function
 */
function commerce_cybersource_fundraiser_commerce_validate($submission_fields, $payment_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $form_parents = array_merge($payment_fields['#parents'], array('credit'));
  _fundraiser_commerce_credit_card_validate($submission_fields, $form_parents);
  return $submission_fields;
}

/**
 * Callback function, charge gateway.
 */
function commerce_cybersource_fundraiser_commerce_charge($payment_method, $donation) {
  // Translate from donation settings to the correct values for the plugin.
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $order = commerce_order_load($donation->did);
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $wrapper->commerce_order_total->value();
  $pane_form = array(); // Not actually used anywhere in this system, so ignore it.

  $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);
  _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation);
  
  // Customer data for charge pulls from the customer profile given on the order, so we can ignore that.
  // If the charge fails, returns false, else nothing.
  $result = commerce_cybersource_soap_cc_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge, $donation);
  $success = FALSE;
  if ($result !== FALSE) {
    $success = TRUE;
  }
  // Perform post processing functions
  _fundraiser_commerce_charge_submit_form_process($success, $payment_method, $pane_values, $donation);
  return $success;
}


/**
 * Card on file callback
 */
function commerce_cybersource_fundraiser_commerce_cardonfile($donation) {
  // Use standard function to setup the card on file fields
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $cardonfile_fields = _fundraiser_commerce_credit_card_cardonfile_fields($donation);

  // We need to search locally to see if we have an existing profile for a card number
  // Only perform this when we're in an active non-reference charge
  if (isset($donation->reference_charge) && $donation->reference_charge === FALSE) {
    // Query the db for an existing record
    $card_data = _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields);

    // If we have a profile for this user update it in the Vault
    if ($card_data) {
      // Make the update request
      commerce_cybersource_fundraiser_commerce_update($donation, $card_data);
    }
  }
  return $cardonfile_fields;
}

/**
 * Callback function, update card data stored at the gateway and locally
 *
 * @param $donation Object
 *    Fundraiser commerce donation object which should include the new/updated card data
 * @param $card_data Object
 *    Card on file entity, this should be the current entity for the donation,
 *    new data should be passed in the donation object
 *
 * @return Object
 *    Card on file entity for the new/updated card data
 */
function commerce_cybersource_fundraiser_commerce_update($donation, $card_data = NULL) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $payment_method = $donation->gateway['gateway_details'];

  // If no card data was passed load the profile from the donation if its set
  if (empty($card_data) && !empty($donation->data['cardonfile'])) {
    $card_data = commerce_cardonfile_load($donation->data['cardonfile']);
  }

  if (empty($card_data)) {
    // Log the missing card data
    watchdog(
      'fundraiser',
      'A card update was attempted on donation @id but no stored card data was found.',
      array('@id' => $donation->did),
      WATCHDOG_CRITICAL
    );
    return $card_data;
  }

  // Pull the new credit card values from the donation
  $pane_values = array('values' => _fundraiser_commerce_credit_card_pane_values($donation));

  // Load the card on file values for this donation
  $cardonfile_fields = _fundraiser_commerce_credit_card_cardonfile_fields($donation);

  // Check the card number to see if it matches the current card data number
  // If it does then we use the update function
  // if its a new card check if we should create a new vault profile
  $action = 'update';
  if ($cardonfile_fields['card_number'] != $card_data->card_number) {
    // This is a new card for this donation, make sure we don't have an existing profile
    // First check with the card number and date
    if ($existing = _fundraiser_commerce_cardonfile_match($cardonfile_fields)) {
      // Set the existing card on the donation and return it
      $donation->data['cardonfile'] = $existing->card_id;
      $card_data = $existing;
    }
    // Check if we have a mismatched exp date record
    elseif ($mismatch =  _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields)) {
      $donation->data['cardonfile'] = $mismatch->card_id;
      $card_data = $mismatch;
    }
    // Nothing stored locally so create a new profile
    else {
      $action = 'new';
    }
  }

  if ($action == 'update') {
    $donation->old_card = clone $card_data;

    $card_data->card_number = $cardonfile_fields['card_number'];
    $card_data->card_exp_month = $cardonfile_fields['card_exp_month'];
    $card_data->card_exp_year = $cardonfile_fields['card_exp_year'];
    $pane_form = array();
    $order = commerce_order_load($donation->did);

    $pane_values['values']['billto'] = commerce_cybersource_address('billing', $order, 'commerce_customer_billing');
    $response = commerce_cybersource_cardonfile_update($pane_form, $pane_values, $payment_method, $card_data);
    if ($response) {
      commerce_cardonfile_save($card_data);
      return $card_data;
    }
  }
  elseif ($action == 'new') {
    $order = commerce_order_load($donation->did);
    $request = new stdClass();
    $request->billTo = commerce_cybersource_address('billing', $order, 'commerce_customer_billing');
    $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);
    $request->card = (object) array(
      'accountNumber' => $pane_values['credit_card']['number'],
      'expirationMonth' => $pane_values['credit_card']['exp_month'],
      'expirationYear' => $pane_values['credit_card']['exp_year'],
    );
    $request->merchantReferenceCode = 'Order ' . $order->order_number;
    $request->purchaseTotals = (object) array('currency' => 'USD');
    $request->subscription = (object) array(
        'title' => 'On-Demand Profile',
        'paymentMethod' => 'credit card'
      );
    $request->recurringSubscriptionInfo = (object) array('frequency' => 'on-demand');
    $request->paySubscriptionCreateService = (object) array('run' => 'true');
    $card_type = commerce_payment_validate_credit_card_type($pane_values['credit_card']['number'], array_keys(commerce_payment_credit_card_types()));
    $request->card->cardType = commerce_cybersource_cc_type_code($card_type);

    if ($response = commerce_cybersource_create_customer_profile_request($request, $payment_method, $order)) {
      // If the Customer Profile creation was a success, store the new card on
      // file data locally.
      if ($response->paySubscriptionCreateReply->reasonCode == 100) {
        // Build a remote ID that includes the MerchantRefernceCode and ProfileSubscriptionID
        $remote_id = $response->paySubscriptionCreateReply->subscriptionID;
            // Get the card type
        $all_types = commerce_payment_credit_card_types();
        $card_type = !empty($all_types[$card_type]) ? $all_types[$card_type] : 'card';
      // Store the token with cardonfile.
        $new_data = commerce_cardonfile_new();
        $new_data->uid = $order->uid;
        $new_data->payment_method = $payment_method['method_id'];
        $new_data->instance_id = $payment_method['instance_id'];
        $new_data->remote_id = $remote_id;
        $new_data->card_type = !empty($card_type) ? $card_type : 'card';
        $new_data->card_name = !empty($card_data->card_name) ? $card_data->card_name : '';
        $new_data->card_number = substr($pane_values['credit_card']['number'], -4);
        $new_data->card_exp_month = $pane_values['credit_card']['exp_month'];
        $new_data->card_exp_year = $pane_values['credit_card']['exp_year'];
        $new_data->status = 1;

        // Save and log the creation of the new card on file.
        commerce_cardonfile_save($new_data);

        $donation->data['cardonfile'] = $new_data->card_id;
        return $new_data;
      }
    }
  }

}

/**
 * Refund callback for cybersource, wraps the admin form used for refund with our own system.
 */
function commerce_cybersource_fundraiser_commerce_refund($payment_method, $refund) {
  // Retrieve data.
  $refunding_order = commerce_order_load($refund->did);
  $transactions = commerce_payment_transaction_load_multiple(array(), $conditions = array('order_id' => $refunding_order->order_id));
  $transaction = array_shift($transactions); // The first one is the original donation.

  $form = array();
  $form_state = array();
  // Set up the values needed for the transaction to go through.
  $form_state['transaction'] = $transaction;
  $form_state['values']['amount'] = $refund->amount;
  $form_state['order'] = $refunding_order;
  $form_state['payment_method'] = commerce_payment_method_instance_load($transaction->instance_id);
  // Submit the donation form.
   module_load_include('inc', 'commerce_cybersource', '/includes/commerce_cybersource.admin');
   $result = commerce_cybersource_soap_refund_form_submit($form, $form_state);

  // At this point the card has been saved on the user profile.
  $success = FALSE;
  if ($result !== FALSE) {
    $success = TRUE;
  }
  return $success;
}
