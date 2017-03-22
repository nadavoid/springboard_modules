<?php

/**
 * @file
 * Commerce based hook for commerce_authnet.
 */

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_braintree_web_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    'payment_method' => array(
      'braintree_hostedfields' => 'credit',
      'braintree_paypal' => 'paypal',
      'braintree_applepay' => 'applepay',
    ),
    'allow_recurring' => array('credit', 'paypal', 'applepay'),
    'allow_refund' => array('credit', 'paypal', 'applepay'),
    'form callback' => 'commerce_braintree_web_fundraiser_commerce_form',
    // 'cleanup callback' => 'commerce_authnet_fundraiser_commerce_cleanup',
    // 'scrub callback' => 'commerce_authnet_fundraiser_commerce_scrub',
    'expire callback' => 'commerce_braintree_web_fundraiser_commerce_expire',
    'validate callback' => 'commerce_braintree_web_fundraiser_commerce_validate',
    'charge callback' => 'commerce_braintree_web_fundraiser_commerce_charge',
    'update callback' => 'commerce_braintree_web_fundraiser_commerce_update',
    'refund callback' => 'commerce_braintree_web_fundraiser_commerce_refund',
    // 'cardonfile callback' => 'commerce_braintree_web_fundraiser_commerce_cardonfile',
    // 'response fields callback' => 'commerce_authnet_fundraiser_commerce_response_fields',
    // 'response save callback' => 'commerce_authnet_fundraiser_commerce_response_save',
  );
}

/**
 * Validate the submitted values with the commerce validate function.
 */
function commerce_braintree_web_fundraiser_commerce_validate($submission_fields, $payment_fields, $payment_method) {
  // Because user input is handled client-side by braintree, all we can and
  // should do is check for the existence of a value in the hidden nonce field.
  commerce_braintree_web_submit_form_validate(NULL, $submission_fields, $payment_fields, NULL);
  return $submission_fields;
}

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_braintree_web_fundraiser_donation_form_config_check($node) {
  $messages = array();
  foreach ($node->gateways as $gateway) {
    if (!empty($gateway['id'])  && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);
      if ($gateway_config['gateway_details']['module'] == 'commerce_braintree_web') {
        if ($gateway_config['gateway_details']['settings']['environment'] == 'sandbox') {
          $messages[] = t('%gateway is currently using the sandbox environment and will not process live transactions.', array('%gateway' => $gateway_config['title']));
        }
      }
    }
  }
  return $messages;
}

/**
 * Card on file callback.
 *
 * We get payment fields from the transaction payload associated with the master
 * donation id, if one is present.
 */
// function commerce_braintree_web_fundraiser_commerce_cardonfile($donation) {
//   $cardonfile_fields = array();

//   if (isset($donation->master_did)) {
//     $master_donation = fundraiser_donation_get_donation($donation->master_did);

//     $payload = db_query("SELECT payload FROM {commerce_payment_transaction} WHERE remote_id = :txn_id LIMIT 1", array(':txn_id' => $master_donation->txn_id))->fetchField();

//     if (isset($payload)) {
//       // Braintree classes appear to not be available during fundraiser_cron.
//       // Load them here so we can use the object in the payload.
//       if (!class_exists('Braintree\Result\Successful')) {
//         libraries_load('braintree_php');
//       }
//       $braintree_response_object = unserialize($payload);
//       $braintree_response_object = reset($braintree_response_object);
//       // $first_key = key($braintree_response_object);
//       // $transaction = $braintree_response_object[$first_key]->transaction;
//       $transaction = $braintree_response_object->transaction;
//       $cardonfile_fields = array(
//         'card_number' => isset($transaction->creditCard['last4']) ? $transaction->creditCard['last4'] : '',
//         'card_exp_month' => isset($transaction->creditCard['expirationMonth']) ? $transaction->creditCard['expirationMonth'] : '',
//         'card_exp_year' => isset($transaction->creditCard['expirationYear']) ? $transaction->creditCard['expirationYear'] : '',
//         'instance_id' => $donation->gateway['id'],
//         'uid' => $donation->uid,
//       );
//     }
//   }
//   return $cardonfile_fields;
// }

/**
 * Callback function, return the user entered experation dates for this submission.
 */
function commerce_braintree_web_fundraiser_commerce_expire($submission_fields) {
  $expires = array();
  // We may already have the expiration dates in the payment_fields if this is an upsell.
  if (isset($submission_fields['payment_fields']['credit']['card_expiration_month']) && !empty($submission_fields['payment_fields']['credit']['card_expiration_month'])) {
    $expires['month'] = $submission_fields['payment_fields']['credit']['card_expiration_month'];
    $expires['year'] = $submission_fields['payment_fields']['credit']['card_expiration_year'];
  }
  // If the cardonfile element is set, load the card and use its expiration
  // month and year values.
  elseif (!empty($submission_fields['cardonfile']) && is_numeric($submission_fields['cardonfile'])) {
    $card = commerce_cardonfile_load($submission_fields['cardonfile']);
    $expires['month'] = $card->card_exp_month;
    $expires['year'] = $card->card_exp_year;
  }
  // If we don't find them in payment_fields, look in the payload returned from
  // the braintree transaction sale.
  elseif (isset($submission_fields['txn_id'])) {
    $payload = db_query("SELECT payload FROM {commerce_payment_transaction} WHERE remote_id = :txn_id LIMIT 1", array(':txn_id' => $submission_fields['txn_id']))->fetchField();
    if (isset($payload)) {
      $braintree_response_object = unserialize($payload);
      reset($braintree_response_object);
      $first_key = key($braintree_response_object);
      $transaction = $braintree_response_object[$first_key]->transaction;
      if (isset($transaction->creditCard['expirationMonth'])) {
        $expires['month'] = $transaction->creditCard['expirationMonth'];
      }
      if (isset($transaction->creditCard['expirationYear'])) {
        $expires['year'] = $transaction->creditCard['expirationYear'];
      }
    }
  }
  // If we could not find expiration month and year in the transaction
  // payload (eg paypal), stub an expiration date 24 months in the future.
  if (!isset($expires['month']) && !isset($expires['year'])) {
    $months = variable_get('commerce_braintree_web_sustainer_series_count', 24);
    $exp_timestamp = strtotime('+' . $months . ' months');
    $expires['month'] = date('m', $exp_timestamp);
    $expires['year'] = date('Y', $exp_timestamp);
  }
  return $expires;
}

/**
 * Refund callback for authnet, wraps the admin form used for refund with our own system.
 */
function commerce_braintree_web_fundraiser_commerce_refund($payment_method, $refund) {
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
  // module_load_include('inc', 'commerce_braintree_web', '/includes/commerce_authnet.admin');
  $result = commerce_braintree_web_refund_form_submit($form, $form_state);
  // At this point the card has been saved on the user profile.
  $success = FALSE;
  if ($result !== FALSE) {
    $success = TRUE;
  }
  return $success;
}

/**
 * Callback function, use this form for the given gateway.
 *
 * @param string $payment_method
 *   The payment method ID.
 * @param array $config
 *   The payment method configuration.
 */
function commerce_braintree_web_fundraiser_commerce_form($payment_method, $config = NULL) {
  $billing_update = FALSE;
  if (empty($config)) {
    // If the $config variable is empty, we assume this function was called from
    // the billing update form.
    $config = array(
      'id' => 'braintree_hostedfields|commerce_payment_braintree_hostedfields',
    );
    $billing_update = TRUE;
  }
  $form = braintree_form_elements($payment_method, $config, $billing_update);
  return $form;
}

/**
 * Callback function, charge gateway.
 */
function commerce_braintree_web_fundraiser_commerce_charge($method_instance, $donation) {
  $order = commerce_order_load($donation->did);
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $wrapper->commerce_order_total->value();

  $pane_form = array();

  // We may need to set these values. They are used in
  // commerce_braintree_web_submit_form_submit() below.
  // $pane_values['braintree']['using_existing_payment_method']
  // $pane_values['braintree']['payment_method_token']
  // $pane_values['braintree']['payment_method_token']
  // $pane_values['braintree']['default_method_token'].

  $pane_values = array();

  _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation);

  $success = FALSE;
  $result = commerce_braintree_web_submit_form_submit($method_instance, $pane_form, $pane_values, $order, $charge);
  if ($result !== FALSE) {
    $success = TRUE;
  }

  if (isset($result->card_id)) {
    $donation->data['cardonfile'] = $result->card_id;
  }

  // Setting payment_fields here, because we don't have this data until after
  // the playload from the braintree sale is returned to us.
  if (isset($result->transaction->paymentInstrumentType) && $result->transaction->paymentInstrumentType == 'credit_card') {
    $donation->data['payment_fields'] = array(
      'credit' => array(
        'card_number' => $result->transaction->creditCard['last4'],
        'card_type' => $result->transaction->creditCard['cardType'],
        'card_expiration_month' => $result->transaction->creditCard['expirationMonth'],
        'card_expiration_year' => $result->transaction->creditCard['expirationYear'],
      ),
    );
  }
  elseif (isset($result->transaction->paymentInstrumentType) && $result->transaction->paymentInstrumentType == 'paypal_account') {
    $donation->data['payment_fields'] = array(
      'paypal' => array(
        'payer_email' => $result->transaction->paypal['payerEmail'],
      ),
    );
  }

  // Adapted from commerce_authnet_echeck.inc.
  // If this is recurring PayPal payment.
  if ($success == TRUE && isset($donation->recurring) && !empty($donation->recurring) && $method_instance['method_id'] == 'braintree_paypal') {
    // Update expiration date and generate the next donation.
    $card_data = _fundraiser_commerce_donation_cardonfile_card($donation);
    if (!empty($card_data) && $donation->status <> 'failed') {
      // If the next recurring series entry doesn't exist, create one.
      if (_fundraiser_sustainers_count_donations_recurr_remaining($donation->recurring->master_did) < 1) {
        // Move card expiration date ahead one month.
        $new_expire_time = mktime(0, 0, 0, ($card_data->card_exp_month + 1), 1, $card_data->card_exp_year);
        $card_data->card_exp_month = date('m', $new_expire_time);
        $card_data->card_exp_year = date('Y', $new_expire_time);
        $submission_fields = array(
          'payment_fields' => array(
            'braintree_paypal' => array(
              'card_expiration_month' => $card_data->card_exp_month,
              'card_expiration_year' => $card_data->card_exp_year,
            ),
          ),
        );
        // Update card data.
        commerce_cardonfile_save($card_data);
        // Create new orders.
        $master_donation = fundraiser_donation_get_donation($donation->recurring->master_did);
        fundraiser_sustainers_update_billing_info_create_new_donations($master_donation, $donation, $submission_fields);
      }
    }
  }
  // Perform post processing functions.
  _fundraiser_commerce_charge_submit_form_process($success, $method_instance, $pane_values, $donation);

  return $success;
}

/**
 * Callback function, update card data stored at the gateway and locally.
 *
 * @param object $donation
*     Fundraiser commerce donation object which should include the new/updated
 *    card data.
 * @param object $card_data
 *    Card on file entity, this should be the current entity for the donation,
 *    new data should be passed in the donation object.
 *
 * @return Object
 *    Card on file entity for the new/updated card data
 */
function commerce_braintree_web_fundraiser_commerce_update($donation, $card_data = NULL) {
  // Update the billing address.
  $result = db_query('SELECT customer_id, address_id, profile_id FROM {braintree_vault_addresses} WHERE payment_method_token = :pmt', array(':pmt' => $card_data->remote_id))->fetchAll();
  $result = array_shift($result);

  if (empty($result)) {
    return;
  }

  $customer_id = $result->customer_id;
  $address_id = $result->address_id;
  $profile_id = $result->profile_id;

  $submission_data = $donation->submission_data;
  $donation_data = $donation->donation;

  $needs_update = FALSE;

  $name_change = FALSE;

  $address_fields = array(
    'first_name',
    'last_name',
    'address',
    'address_line_2',
    'city',
    'country',
    'state',
    'zip',
  );
  $address_change = FALSE;
  foreach ($address_fields as $af) {
    if ($donation_data[$af] != $submission_data[$af]) {
      if ($af == 'first_name' || $af == 'last_name') {
        $name_change = TRUE;
      }
      $address_change = TRUE;
      break;
    }
  }

  $options = array();
  $address_data = array(
    'firstName' => $donation_data['first_name'],
    'lastName' => $donation_data['last_name'],
    'streetAddress' => $donation_data['address'],
    'extendedAddress' => $donation_data['address_line_2'],
    'locality' => $donation_data['city'],
    'region' => $donation_data['state'],
    'postalCode' => $donation_data['zip'],
    'countryCodeAlpha2' => $donation_data['country'],
  );

  // If the addresses don't match call the API to update.
  if ($address_change) {
    $result = braintree_vault_address_update($customer_id, $address_id, $address_data, $donation->gateway['gateway_details']['settings']);

    $options['billingAddressId'] = $address_id;

    $needs_update = TRUE;
  }

  // If the nonce is set, set it in the options array and flag for update.
  $nonce = braintree_get_nonce();
  // If $nonce is non-empty, the user entered new card details.
  if (!empty($nonce)) {
    $options['paymentMethodNonce'] = $nonce;

    // In order to update payment information, street address and postal code
    // also need to be passed. So set the billing address information if the
    // billingAddressId isn't set.
    if (empty($options['billingAddressId'])) {
      $options['billingAddress'] = $address_data;
    }
    $needs_update = TRUE;
  }
  else {
    // Otherwise the nonce wasn't set, so return now.
    return $card_data;
  }

  // Update the payment information.
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $payment_method = $donation->gateway['gateway_details'];

  // If no card data was passed load the profile from the donation if it's set.
  if (empty($card_data) && !empty($donation->data['cardonfile'])) {
    $card_data = commerce_cardonfile_load($donation->data['cardonfile']);
  }

  if (empty($card_data)) {
    // Log the missing card data.
    watchdog('fundraiser', 'A card update was attempted on donation @id but no stored card data was found.', array('@id' => $donation->did), WATCHDOG_CRITICAL);
    return;
  }

  if ($name_change) {
    $options['cardholderName'] = $donation_data['first_name'] . ' ' . $donation_data['last_name'];
    $needs_update = TRUE;
  }

  if ($needs_update) {
    $donation->old_card = clone $card_data;

    list($success, $context) = braintree_vault_payment_method_save(array(
      'paymentMethodToken' => $card_data->remote_id,
    ), array(
      'op' => 'edit',
      'options' => $options,
    ), $donation->gateway['gateway_details']['settings']);

    // Update cardonfile record.
    // Get the card type. We could just use the type from the result, but in
    // order to be more consistent with other payment methods and ensure
    // consistent card type values, we use this roundabout way of determining
    // the type.
    module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
    $card_type = commerce_payment_validate_credit_card_type($context['result']->paymentMethod->bin, array_keys(commerce_payment_credit_card_types()));
    $all_types = commerce_payment_credit_card_types();
    // If the card type isn't found, use the card type from the response object
    // if it's set.
    $card_type = !empty($all_types[$card_type]) ? $all_types[$card_type] : !empty($context['result']->paymentMethod->cardType) ? $context['result']->paymentMethod->cardType : 'card';

    // Update cardonfile entity.
    $card_data->payment_method = $payment_method['method_id'];
    $card_data->instance_id = $payment_method['instance_id'];
    $card_data->card_type = $card_type;
    $card_data->card_name = !empty($context['result']->paymentMethod->cardholderName) ? $context['result']->paymentMethod->cardholderName : $context['result']->paymentMethod->billingAddress->firstName . ' ' . $context['result']->paymentMethod->billingAddress->lastName;
    $card_data->card_number = $context['result']->paymentMethod->last4;
    $exp_month_different = $context['result']->paymentMethod->expirationMonth != $card_data->card_exp_month;
    $card_data->card_exp_month = $context['result']->paymentMethod->expirationMonth;
    $exp_year_different = $context['result']->paymentMethod->expirationYear != $card_data->card_exp_year;
    $card_data->card_exp_year = $context['result']->paymentMethod->expirationYear;
    $card_data->status = 1;

    // Update {braintree_vault_payment_methods} record.
    db_merge('braintree_vault_payment_methods')
      ->key(array(
        'payment_method_token' => $context['result']->paymentMethod->token,
        'card_id' => $card_data->card_id,
      ))
      ->fields(array(
        'uid' => $card_data->uid,
        'customer_id' => $context['result']->paymentMethod->customerId,
        'payment_method_token' => $context['result']->paymentMethod->token,
        'card_id' => $card_data->card_id,
        'unique_identifier' => $context['result']->paymentMethod->uniqueNumberIdentifier,
      ))
      ->execute();

    db_merge('braintree_vault_addresses')
      ->key(array(
        'customer_id' => $context['result']->paymentMethod->customerId,
        'address_id' => $address_id,
      ))
      ->fields(array(
        'customer_id' => $context['result']->paymentMethod->customerId,
        'address_id' => $address_id,
        'payment_method_token' => $context['result']->paymentMethod->token,
        'uid' => $card_data->uid,
        'profile_id' => $profile_id,
      ))
      ->execute();

    // Check if the expiration date changed. If it has and this is a recurring
    // series, we need to add the submission handler from the
    // fundraiser_sustainers module to the submission handler array.
    if (empty($donation->update_from_sustainer) && module_exists('fundraiser_sustainers') && property_exists($donation, 'recurring')) {
      if ($exp_month_different || $exp_year_different) {
        $master_donation = fundraiser_donation_get_donation($donation->recurring->master_did);
        fundraiser_sustainers_update_billing_info_create_new_donations($master_donation, $donation, $donation->submission_data);
      }
    }

    // Save and log the creation of the new card on file.
    commerce_cardonfile_save($card_data);
    watchdog('commerce_braintree_web', 'Customer Profile @profile_id updated and saved to user @uid.', array('@profile_id' => (string) $context['result']->paymentMethod->billingAddress->customerId, '@uid' => $card_data->uid));
  }

  return $card_data;
}
