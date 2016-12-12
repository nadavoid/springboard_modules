<?php

/**
 * @file
 * Commerce based hook for commerce_authnet EFT.
 */

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_authnet_echeck_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    'payment_method' => array('bank account'),
    'allow_recurring' => array('bank account'),
    'form callback' => 'commerce_authnet_echeck_fundraiser_commerce_submit_form',
    'cleanup callback' => 'commerce_authnet_echeck_fundraiser_commerce_cleanup',
    'scrub callback' => 'commerce_authnet_echeck_fundraiser_commerce_scrub',
    'validate callback' => 'commerce_authnet_echeck_fundraiser_commerce_validate',
    'expire callback' => 'commerce_authnet_echeck_fundraiser_commerce_expire',
    'charge callback' => 'commerce_authnet_echeck_fundraiser_commerce_charge',
    'update callback' => 'commerce_authnet_echeck_fundraiser_commerce_update',
    'cardonfile callback' => 'commerce_authnet_echeck_fundraiser_commerce_cardonfile',
  );
}

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_authnet_echeck_fundraiser_donation_form_config_check($node) {
  foreach ($node->gateways as $method => $gateway) {
    if (!empty($gateway['id']) && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);
      if ($gateway_config['gateway_details']['method_id'] == 'authnet_echeck' &&
        $gateway_config['gateway_details']['settings']['txn_mode'] != AUTHNET_TXN_MODE_LIVE) {
        return array(t('The authnet EFT payment gateway is currently in !mode mode and will not process live transactions.', array('!mode' => $gateway_config['gateway_details']['settings']['txn_mode'])));
      }
    }
  }
}

function commerce_authnet_echeck_fundraiser_gateway_status_cron_check($details) {
  if ($details['method_id'] == 'authnet_echeck') {
    if ($details['settings']['txn_mode'] != AUTHNET_TXN_MODE_LIVE) {
      return $details['settings']['txn_mode'];
    }
    else {
      return 'live';
    }
  }
}

/**
 * Returns the form fields for this method.
 *
 * @param string $payment_method
 *   Credit, bank account, paypal, etc.
 * @param array $config
 *   Configuration options for this field.
 */
function commerce_authnet_echeck_fundraiser_commerce_submit_form($payment_method) {
  module_load_include('inc', 'commerce_authnet_echeck', 'includes/commerce_authnet_echeck.echeck');
  $form = commerce_authnet_echeck_submit_form($payment_method);
  // Remove the required flag for the fields
  foreach (element_children($form['bank account']) as $field_name) {
    $form['bank account'][$field_name]['#required'] = FALSE;
  }
  return $form['bank account'];
}

/**
 * Callback function, scrub the data before saving.
 */
function commerce_authnet_echeck_fundraiser_commerce_scrub($fields) {
  // Scrub sensitive donation fields if they exists.
  if (isset($fields['bank account']['aba_code'])) {
    $fields['bank account']['aba_code'] = substr($fields['bank account']['aba_code'], -3);
  }
  if (isset($fields['bank account']['acct_num'])) {
    $fields['bank account']['acct_num'] = substr($fields['bank account']['acct_num'], -3);
  }
  // Unset confirmation for account number
  unset($fields['bank account']['confirm_acct_num']);
  return $fields;
}

/**
 * Validate the submitted values with the authnet_commerce validate function.
 */
function commerce_authnet_echeck_fundraiser_commerce_validate($submission_fields, $payment_fields) {
  module_load_include('inc', 'commerce_authnet_echeck', 'includes/commerce_authnet_echeck.echeck');
  $form_parents = array_merge($payment_fields['#parents'], array('bank account'));
  $values = array('bank account' => $submission_fields['payment_fields']['bank account']);
  commerce_authnet_echeck_submit_form_validate(NULL, $payment_fields, $values, NULL, $form_parents);
  return $submission_fields;
}

/**
 * Submit the donation values to the Authnet Echeck charge handler.
 *
 * @param array $method_instance
 *   Commerce loaded method instance.
 * @param object $donation
 *   The donation to charge.
 *
 * @return bool
 *   Whether the charge was successful.
 */
function commerce_authnet_echeck_fundraiser_commerce_charge($method_instance, $donation) {
  module_load_include('inc', 'commerce_authnet_echeck', 'includes/commerce_authnet_echeck.echeck');
  // Translate from donation settings to the correct values for the plugin.
  $order = commerce_order_load($donation->did);
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $wrapper->commerce_order_total->value();
  // Not actually used anywhere in this system, so ignore it.
  $pane_form = array();
  $pane_values = commerce_authnet_echeck_pane_values($donation);

  // Add fundraiser commerce data to the pane values array.
  _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation, 'bank account');

  // Execute call to Authnet, only returns FALSE if the payment failed,
  // returns nothing if successful.
  $success = FALSE;
  $result = commerce_authnet_echeck_submit_form_submit($method_instance, $pane_form, $pane_values, $order, $charge);
  if ($result !== FALSE) {
    $success = TRUE;
  }

  // Perform post processing functions.
  _fundraiser_commerce_charge_submit_form_process($success, $method_instance, $pane_values, $donation, 'bank account');

  if ($success == TRUE && isset($donation->recurring) && !empty($donation->recurring)) {
    // Update expiration date and generate the next donation.
    $cardonfile_fields = commerce_authnet_echeck_cardonfile_fields($donation);
    $card_data = _fundraiser_commerce_cardonfile_match($cardonfile_fields);

    if (!empty($card_data) && $donation->status <> 'failed') {
      // If the next recurring series entry doesn't exist, create one.
      if (_fundraiser_sustainers_count_donations_recurr_remaining($donation->recurring->master_did) < 1) {

        // Move card expiration date ahead one month.
        $new_expire_time = mktime(0, 0, 0, ($card_data->card_exp_month + 1), 1, $card_data->card_exp_year);

        $card_data->card_exp_month = date('m', $new_expire_time);
        $card_data->card_exp_year = date('Y', $new_expire_time);

        $submission_fields = array(
          'payment_fields' => array(
            'bank account' => array(
              'card_expiration_month' => $card_data->card_exp_month,
              'card_expiration_year' => $card_data->card_exp_year,
            ),
          ),
        );

        // Update card data
        commerce_cardonfile_save($card_data);

        // Create new orders
        $master_donation = fundraiser_donation_get_donation($donation->recurring->master_did);
        fundraiser_sustainers_update_billing_info_create_new_donations($master_donation, $donation, $submission_fields);
      }
    }
  }

  return $success;
}


/**
 * Card on file callback
 *
 * We use a custom callback since we may need to update an existing CIM profile
 */
function commerce_authnet_echeck_fundraiser_commerce_cardonfile($donation) {
  $cardonfile_fields = commerce_authnet_echeck_cardonfile_fields($donation);

  return $cardonfile_fields;
}

/**
 * Expiration date callback.
 */
function commerce_authnet_echeck_fundraiser_commerce_expire($submission_fields) {
  $expires = array();

  if (!empty($submission_fields) && array_key_exists('card_expiration_month', $submission_fields['payment_fields']['bank account']) && array_key_exists('card_expiration_year', $submission_fields['payment_fields']['bank account'])) {
    // Donation passed
    $expires['month'] = $submission_fields['payment_fields']['bank account']['card_expiration_month'];
    $expires['year'] = $submission_fields['payment_fields']['bank account']['card_expiration_year'];
  }
  else {
    // TODO this is causing a problem on billing updates, wherein future orders are deleted
    // If no expiration date given, use current date + 1 month
    $expire_time = strtotime("now + 1 month");

    $expires['month'] = date('m', $expire_time);
    $expires['year'] = date('Y', $expire_time);
  }

  return $expires;
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
function commerce_authnet_echeck_fundraiser_commerce_update($donation, $card_data = NULL) {
  $payment_method = $donation->gateway['gateway_details'];

  // If no card data was passed load the profile from the donation if its set
  if (empty($card_data) && !empty($donation->data['cardonfile'])) {
    $card_data = commerce_cardonfile_load($donation->data['cardonfile']);
  }
  elseif (empty($card_data)) {
    // If there wasn't card data on the donation, try to load it for user/payment method
    $cardonfile_fields = commerce_authnet_echeck_cardonfile_fields($donation);
    $card_data = _fundraiser_commerce_cardonfile_match($cardonfile_fields);
  }

  // Pull the new credit card values from the donation
  $pane_values = commerce_authnet_echeck_pane_values($donation);

  // Load the card on file values for this donation
  $cardonfile_fields = commerce_authnet_echeck_cardonfile_fields($donation);

  // Check the card number to see if it matches the current card data number
  // If it does then we use the update function
  // if its a new card check if we should create a new CIM profile
  $action = 'update';
  if (!isset($card_data) || $cardonfile_fields['card_number'] != $card_data->card_number) {
    // This is a new card for this donation, make sure we don't have an existing profile
    // First check with the card number and date
    if ($existing = _fundraiser_commerce_cardonfile_match($cardonfile_fields)) {
      // Set the existing card on the donation and return it
      $donation->data['cardonfile'] = $existing->card_id;
      $card_data = $existing;
    }
    // Nothing stored locally so create a new profile
    else {
      $action = 'new';
    }
  }
  else {
    // If cardonfile matched, set it here
    $donation->data['cardonfile'] = $card_data->card_id;
  }

  if ($action == 'update') {
    $card_data->card_number = $cardonfile_fields['card_number'];

    // We're mirroring the commerce_authnet_cim_cardonfile_update function so setup values that function uses, see that function for details
    $form['bank account']['acct_num']['#default_value'] = '';
    $form_state['values']['bank account']['acct_num'] = $pane_values['bank account']['acct_num'];
    $form_state['values']['bank account']['aba_code'] = $pane_values['bank account']['aba_code'];
    $form_state['values']['bank account']['acct_name'] = $pane_values['bank account']['acct_name'];
    $form_state['values']['bank account']['owner'] = isset($donation->donation['first_name']) ? $donation->donation['first_name'] : '';
    $form_state['values']['bank account']['owner'] .= isset($donation->donation['last_name']) ? ' ' . $donation->donation['last_name'] : '';
    $form_state['values']['bank account']['address'] = isset($donation->donation['address']) ? $donation->donation['address'] : '';
    $form_state['values']['bank account']['address'] .= isset($donation->donation['address_line_2']) ? ' ' . $donation->donation['address_line_2'] : '';
    $form_state['values']['bank account']['city'] = isset($donation->donation['city']) ? $donation->donation['city'] : '';
    $form_state['values']['bank account']['state'] = isset($donation->donation['state']) ? $donation->donation['state'] : '';
    $form_state['values']['bank account']['zip'] = isset($donation->donation['zip']) ? $donation->donation['zip'] : '';
    $form_state['values']['bank account']['country'] = isset($donation->donation['country']) ? $donation->donation['country'] : '';

    $response = _commerce_authnet_echeck_fundraiser_commerce_authnet_cim_cardonfile_update($form, $form_state, $payment_method, $card_data);

    if ($response) {
      $donation->data['cardonfile'] = $card_data->card_id;
      commerce_cardonfile_save($card_data);
      return $card_data;
    }
  }
  // Create a new card on file record
  elseif ($action == 'new') {
    // We're using the built in function that requires a payment details array and the order
    $payment_details = array(
      'routingNumber' => $pane_values['bank account']['aba_code'],
      'accountNumber' => $pane_values['bank account']['acct_num'],
      'nameOnAccount' => $pane_values['bank account']['acct_name']
    );

    // Load the order
    $order = commerce_order_load($donation->did);
    $response = commerce_authnet_echeck_cim_create_customer_profile_request($payment_method, $order, $payment_details);

    // If the Customer Profile creation was a success, store the new card on file data locally.
    if ($response && (string) $response->messages->resultCode == 'Ok') {
      // Build a remote ID that includes the Customer Profile ID and the
      // Payment Profile ID.
      $remote_id = (string) $response->customerProfileId . '|' . (string) $response->customerPaymentProfileIdList->numericString;

      // Get the card type
      $card_type = 'card';

      // Get card expiration date to use if we don't have one
      $expires = commerce_authnet_echeck_fundraiser_commerce_expire(array());

      // Create a new cardonfile entity
      $new_data = commerce_cardonfile_new();
      $new_data->uid = $order->uid;
      $new_data->payment_method = $payment_method['method_id'];
      $new_data->instance_id = $payment_method['instance_id'];
      $new_data->remote_id = $remote_id;
      $new_data->card_type = $card_type;
      $new_data->card_name = !empty($card_data->card_name) ? $card_data->card_name : '';
      $new_data->card_number = $cardonfile_fields['card_number'];
      // We want to use the exp date and month from the old card, so that sustainers won't delete any orders
      $new_data->card_exp_month = !empty($card_data->card_exp_month) ? $card_data->card_exp_month : $expires['month'];
      $new_data->card_exp_year = !empty($card_data->card_exp_year) ? $card_data->card_exp_year : $expires['year'];
      $new_data->status = 1;

      // Save and log the creation of the new card on file.
      commerce_cardonfile_save($new_data);
      // Set the value on the donation
      $donation->data['cardonfile'] = $new_data->card_id;
      watchdog('commerce_authnet', 'CIM Customer Profile @profile_id created and saved to user @uid.', array('@profile_id' => (string) $response->customerProfileId, '@uid' => $order->uid));
      return $new_data;
    }
  }
}

/**
 * COPY: commerce_authnet_cim_cardonfile_update
 * Card on file callback: updates the associated customer payment profile.
 *
 * The original function at commerce_authnet_cim_cardonfile_update(), doesn't update the user
 * address fields - instead referencing the cim values saved, so not even updating the customer profile
 * in advance would do it. So we need to copy this over and make the changes we need for fall through.
 */
function _commerce_authnet_echeck_fundraiser_commerce_authnet_cim_cardonfile_update($form, &$form_state, $payment_method, $card_data) {
  // Extract the Customer Profile and Payment Profile IDs from the remote_id.
  list($cim_customer_profile_id, $cim_payment_profile_id) = explode('|', $card_data->remote_id);

  if ($form_state['values']['bank account']['acct_num'] != $form['bank account']['acct_num']['#default_value']) {
    $number = $form_state['values']['bank account']['acct_num'];
  }
  else {
    $number = 'XXXX' . $card_data->card_number;
  }

  // Load the payment profile so that billTo can be updated.
  $payment_profile_xml = commerce_authnet_cim_get_customer_payment_profile_request($payment_method, $cim_customer_profile_id, $cim_payment_profile_id);
  $billto = $payment_profile_xml->paymentProfile->billTo;
  $first_name = (string) $billto->firstName;
  $last_name = (string) $billto->lastName;
  $address = (string) $billto->address;
  $city = (string) $billto->city;
  $state = (string) $billto->state;
  $zip = (string) $billto->zip;
  $country = (string) $billto->country;

  // Extract the first and last name from form values.
  if (!empty($form_state['values']['bank account']['owner'])) {
    $name_parts = explode(' ', $form_state['values']['bank account']['owner']);
    $first_name = array_shift($name_parts);
    $last_name = implode(' ', $name_parts);
    $address = $form_state['values']['bank account']['address'];
    $city = $form_state['values']['bank account']['city'];
    $state = $form_state['values']['bank account']['state'];
    $zip = $form_state['values']['bank account']['zip'];
    $country = $form_state['values']['bank account']['country'];
  }

  // Build the base profile update data.
  $api_request_data = array(
    'customerProfileId' => $cim_customer_profile_id,
    'paymentProfile' => array(
      'billTo' => array(
        'firstName' => substr($first_name, 0, 50),
        'lastName' => substr($last_name, 0, 50),
        'company' => (string) $billto->company,
        'address' => $address,
        'city' => $city,
        'state' => $state,
        'zip' => $zip,
        'country' => $country,
      ),
      'payment' => array(
        'bankAccount' => array(
          'routingNumber' => $form_state['values']['bank account']['aba_code'],
          'accountNumber' => $form_state['values']['bank account']['acct_num'],
          'nameOnAccount' => $form_state['values']['bank account']['acct_name']
        ),
      ),
      'customerPaymentProfileId' => $cim_payment_profile_id,
    ),
  );

  // Fetch the response from the API server and let Card on File know if the update was successful.
  $xml_response = commerce_authnet_cim_request($payment_method, 'updateCustomerPaymentProfileRequest', $api_request_data);
  return (string) $xml_response->messages->message->code == 'I00001';
}

/**
 * Get Authorize.net eCheck pane values from a donation.
 *
 * @param object $donation
 *   The donation to grab the pane values from.
 *
 * @return array
 *   The pane values.
 */
function commerce_authnet_echeck_pane_values($donation) {
  $pane_values = array();

  if (isset($donation) && isset($donation->donation['payment_fields']['bank account'])) {
    $pane_values['bank account'] = $donation->donation['payment_fields']['bank account'];
  }

  return $pane_values;
}

/**
 * Returns the values need to load a card on file profile for Authnet eCheck.
 *
 * @param object $donation
 *   Fundraiser donation object.
 *
 * @return array
 *   Values ready to be passed into a card on file profile query.
 */
function commerce_authnet_echeck_cardonfile_fields($donation) {
  $pane_values = commerce_authnet_echeck_pane_values($donation);

  $data = array(
    'card_number' => substr($pane_values['bank account']['acct_num'], -3),
  );

  return $data + array('instance_id' => $donation->gateway['id'], 'uid' => $donation->uid);
}

/**
 * Submits a createCustomerProfileRequest XML CIM API request to Authorize.Net.
 *
 * This function will attempt to create a CIM Customer Profile and a default
 * Payment Profile for it using the given payment details.
 *
 * @param $payment_method
 *   The payment method instance array containing the API credentials for a CIM
 *   enabled Authorize.Net account.
 * @param $order
 *   The order object containing the billing address and e-mail to use for the
 *   customer profile.
 * @param $payment_details
 *   An array of payment details to use in the default payment profile. See the
 *   respective helper array functions for possible keys.
 *
 * @return
 *   A SimpleXMLElement containing the API response.
 *
 * @see commerce_authnet_cim_credit_card_array()
 */
function commerce_authnet_echeck_cim_create_customer_profile_request($payment_method, $order, $payment_details) {
  $billto = commerce_authnet_cim_billto_array($order);

  // Build the base profile request data.
  $api_request_data = array(
    'profile' => array(
      'merchantCustomerId' => $order->uid,
      'description' => $billto['firstName'] . ' ' . $billto['lastName'],
      'email' => $order->mail,
      'paymentProfiles' => array(
        'billTo' => $billto,
        'payment' => array(),
      ),
    ),
  );

  // If the order is anonymous, unset the merchantCustomerId from the request.
  if (empty($api_request_data['profile']['merchantCustomerId'])) {
    unset($api_request_data['profile']['merchantCustomerId']);
  }

  // Add bank account payment details to the default payment profile if given.
  $bank_account = commerce_authnet_echeck_cim_bank_account_array($payment_details);

  if (!empty($bank_account)) {
    $api_request_data['profile']['paymentProfiles']['payment']['bankAccount'] = $bank_account;
  }

  return commerce_authnet_cim_request($payment_method, 'createCustomerProfileRequest', $api_request_data);
}

/**
 * Generates a bankAccount array for CIM API requests.
 *
 * @param $payment_details
 *   An array of payment details used in a CIM API request that doesn't have to
 *   include bank card data. If it does, the following keys are expected:
 *   - routingNumber: The routing number of the customer’s bank.
 *   - accountNumber: The customer’s bank account number.
 *   - nameOnAccount: The customer’s full name as listed on the bank account.
 *
 *   The following options are optional and currently not supported:
 *   - accountType: The type of bank account for the payment profile
 *   - echeckType: The type of electronic check transaction.
 *   - bankName: The name of the bank associated with the bank account number.
 *
 * @see http://www.authorize.net/content/dam/authorize/documents/CIM_XML_guide.pdf
 */
function commerce_authnet_echeck_cim_bank_account_array($payment_details) {
  $bank_account = array();

  foreach (array('routingNumber', 'accountNumber', 'nameOnAccount') as $key) {
    if (!empty($payment_details[$key])) {
      $bank_account[$key] = $payment_details[$key];
    }
  }

  return $bank_account;
}
