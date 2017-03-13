<?php

/**
 * @file
 * Commerce Authorize.Net eCheck Payment Methods.
 */

/**
 * Payment method callback: settings form.
 */
function commerce_authnet_echeck_settings_form($settings = NULL) {
  $form = array();

  // Merge default settings into the stored settings array.
  $settings = (array) $settings + array(
    'login' => '',
    'tran_key' => '',
    'email_customer' => FALSE,
    'txn_mode' => AUTHNET_TXN_MODE_LIVE_TEST,
    'log' => array('request' => '0', 'response' => '0'),
    'cardonfile' => FALSE
  );

  $form['login'] = array(
    '#type' => 'textfield',
    '#title' => t('API Login ID'),
    '#description' => t('Your API Login ID is different from the username you
       use to login to your Authorize.Net account. Once you login, browse to
       your Account tab and click the API Login ID and Transaction Key link to
       find your API Login ID. If you are using a new Authorize.Net account,
       you may still need to generate an ID.'),
    '#default_value' => $settings['login'],
    '#required' => TRUE,
  );

  $form['tran_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Transaction Key'),
    '#description' => t('Your Transaction Key can be found on the same screen as
      your API Login ID. However, it will not be readily displayed. You must answer
      your security question and submit a form to see your Transaction Key.'),
    '#default_value' => $settings['tran_key'],
    '#required' => TRUE,
  );

  $form['txn_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Transaction Mode'),
    '#description' => t('Adjust to live transactions when you are ready to start
      processing real payments.<br />Only specify a developer test account if
      you login to your account through https://test.authorize.net.'),
    '#options' => array(
      AUTHNET_TXN_MODE_LIVE => t('Live transactions in a live account'),
      AUTHNET_TXN_MODE_LIVE_TEST => t('Test transactions in a live account'),
      AUTHNET_TXN_MODE_DEVELOPER => t('Developer test account transactions'),
    ),
    '#default_value' => $settings['txn_mode'],
  );

  // Transaction type is needed for CIM cardonfile transactions. We always want auth/capture.
  $form['txn_type'] = array(
    '#type' => 'hidden',
    '#value' => COMMERCE_CREDIT_AUTH_CAPTURE,
  );

  $form['log'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Log the following messages for debugging'),
    '#options' => array(
      'request' => t('API request messages'),
      'response' => t('API response messages'),
    ),
    '#default_value' => $settings['log'],
  );

  // CIM support in conjunction with AIM requires the Card on File module.
  if (module_exists('commerce_cardonfile')) {
    $form['cardonfile'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable Card on File functionality with this payment method using Authorize.Net CIM.'),
      '#description' => t('This requires an Authorize.Net account upgraded to include support for CIM (Customer Information Manager).'),
      '#default_value' => $settings['cardonfile'],
    );
  }

  else {
    $form['cardonfile'] = array(
      '#type' => 'markup',
      '#markup' => t('To enable Card on File funcitionality download and install the Card on File module.'),
    );
  }

  $form['email_customer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Tell Authorize.net to e-mail the customer a receipt based on your account settings.'),
    '#default_value' => $settings['email_customer'],
  );

  return $form;
}

/**
 * Payment method callback: checkout form.
 */
function commerce_authnet_echeck_submit_form($payment_method, $pane_values = array(), $checkout_pane = NULL, $order = NULL) {
  $fields = drupal_map_assoc(array('bank_name', 'acct_name'));
  $fields['type'] = drupal_map_assoc(array(
    'checking',
    'savings',
  ));

  // Merge default values into the default array.
  $default = array(
    'aba_code' => '',
    'acct_num' => '',
    'confirm_acct_num' => '',
    'type' => '',
    'bank_name' => '',
    'acct_name' => '',
    'code' => '',
  );

  $form['bank account'] = array(
    '#tree' => TRUE,
  );

  // Always add a field for the eCheck routing number.
  $form['bank account']['aba_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Routing Number'),
    '#default_value' => $default['aba_code'],
    '#attributes' => array('autocomplete' => 'off'),
    '#required' => TRUE,
    '#maxlength' => 9,
    '#size' => 10,
  );

  // Always add a field for the eCheck account number and confirmation.
  $form['bank account']['acct_num'] = array(
    '#type' => 'textfield',
    '#title' => t('Account Number'),
    '#default_value' => $default['acct_num'],
    '#attributes' => array('autocomplete' => 'off'),
    '#required' => TRUE,
    '#maxlength' => 20,
    '#size' => 21,
  );
  $form['bank account']['confirm_acct_num'] = array(
    '#type' => 'textfield',
    '#title' => t('Confirm Account Number'),
    '#default_value' => $default['confirm_acct_num'],
    '#attributes' => array('autocomplete' => 'off'),
    '#required' => TRUE,
    '#maxlength' => 20,
    '#size' => 21,
  );

  // Add an account type selector if specified.
  if (isset($fields['type'])) {
    $form['bank account']['type'] = array(
      '#type' => 'select',
      '#title' => t('Account Type'),
      '#options' => array_intersect_key(commerce_authnet_echeck_payment_types(), drupal_map_assoc((array) $fields['type'])),
      '#default_value' => $default['type'],
      '#required' => TRUE,
    );
    $form['bank account']['valid_types'] = array(
      '#type' => 'value',
      '#value' => $fields['type'],
    );
  }
  else {
    $form['bank account']['valid_types'] = array(
      '#type' => 'value',
      '#value' => array(),
    );
  }

  // Add a field for the bank name if specified.
  if (isset($fields['bank_name'])) {
    $form['bank account']['bank_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Bank Name'),
      '#default_value' => $default['bank_name'],
      '#attributes' => array('autocomplete' => 'off'),
      '#required' => TRUE,
      '#maxlength' => 64,
      '#size' => 32,
    );
  }

  // Add a field for the account owner if specified.
  if (isset($fields['acct_name'])) {
    $form['bank account']['acct_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Account Owner'),
      '#default_value' => $default['acct_name'],
      '#attributes' => array('autocomlete' => 'off'),
      '#required' => TRUE,
      '#maxlength' => 64,
      '#size' => 32,
    );
  }

  return $form;
}

/**
 * Payment method callback: checkout form validation.
 */
function commerce_authnet_echeck_submit_form_validate($payment_method, $pane_form, $pane_values, $order, $form_parents = array()) {
  // Validate the echeck fields.
  $prefix = implode('][', $form_parents) . '][';

  // Validate the aba (routing) number.
  if (!commerce_authnet_echeck_payment_validate_aba($pane_values['bank account']['aba_code'])) {
    form_set_error($prefix . 'aba_code', t('You have entered an invalid routing number.'));
  }

  // Ensure that the account numbers match.
  if ($pane_values['bank account']['acct_num'] != $pane_values['bank account']['confirm_acct_num']) {
    form_set_error($prefix . 'acct_num', t('The account numbers do not match.'));
    form_set_error($prefix . 'confirm_acct_num');
  }

  // Validate the account number.
  if (!commerce_authnet_echeck_payment_validate_account($pane_values['bank account']['acct_num'])) {
    form_set_error($prefix . 'acct_num', t('You have entered an invalid account number.'));
  }
}

/**
 * Payment method callback: checkout form submission.
 */
function commerce_authnet_echeck_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {
  // If the customer specified payment using a card on file, attempt that now
  // and simply return the result.
  if (module_exists('commerce_cardonfile') && $payment_method['settings']['cardonfile'] &&
    !empty($pane_values['cardonfile']) && $pane_values['cardonfile'] !== 'new') {
    return commerce_authnet_echeck_cim_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge);
  }

  // Build a name-value pair array for this transaction.
  $nvp = array(
    'x_method' => 'ECHECK',
    'x_bank_aba_code' => $pane_values['bank account']['aba_code'],
    'x_bank_acct_num' => $pane_values['bank account']['acct_num'],
    'x_bank_acct_type' => isset($pane_values['bank account']['type']) ? str_replace('', '_', strtoupper($pane_values['bank account']['type'])) : '',
    'x_bank_name' => isset($pane_values['bank account']['bank_name']) ? substr($pane_values['bank account']['bank_name'], 0, 50) : '',
    'x_bank_acct_name' => isset($pane_values['bank account']['bank_name']) ? substr($pane_values['bank account']['acct_name'], 0, 50) : '',
    'x_echeck_type' => 'WEB',
    // Recurring should always be false, recurring transactions are handled by CIM
    'x_recurring_billing' => 'FALSE',
    'x_amount' => commerce_currency_amount_to_decimal($charge['amount'], $charge['currency_code']),
  );

  // Add additional transaction information to the request array.
  commerce_authnet_echeck_request_order_details($nvp, $order);

  // Submit the request to Authorize.Net.
  $response = commerce_authnet_aim_request($payment_method, $nvp);

  // Prepare a transaction object to log the API response.
  $transaction = commerce_payment_transaction_new($payment_method['method_id'], $order->order_id);
  $transaction->instance_id = $payment_method['instance_id'];
  $transaction->remote_id = $response[6];
  $transaction->amount = $charge['amount'];
  $transaction->currenct_code = $charge['currency_code'];
  $transaction->payload[REQUEST_TIME] = $response;

  // Set transaction status from response code.
  if ($response[0] == '1') {
    $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
    $reason_text = t('APPROVED');
  }
  elseif ($response[0] == '4') {
    $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
    $reason_text = t('PENDING');
  }
  else {
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
    $reason_text = t('REJECTED');
  }

  // Store the type of transaction in the remote status.
  $transaction->remote_status = $response[11];

  // Build a meaningful response message.
  $message = array(
    '<b>ECHECK</b>',
    '<b>' . $reason_text . ':</b> ' . check_plain($response[3]),
  );

  // Add the CVV response if enabled.
  if (!empty($response[6])) {
    $message[] = t('TransactionID: @txn_id', array('@txn_id' => $response[6]));
  }

  $transaction->message = implode('<br />', $message);

  // Save the transaction information.
  commerce_payment_transaction_save($transaction);

  // If the payment failed, display an error and rebuild the form.
  if ($response[0] == '4') {
    drupal_set_message(t('We received the following notice processing your eCheck. Please enter your information again or try a different account.'), 'warning');
    drupal_set_message(check_plain($response[3]), 'warning');
    return FALSE;
  }
  elseif ($response[0] != '1') {
    drupal_set_message(t('We received the following error processing your eCheck. Please enter your information again or try a different account.'), 'error');
    drupal_set_message(check_plain($response[3]), 'error');
    return FALSE;
  }

  // If Card on File storage is enabled via CIM and the form says to store data...
  if (module_exists('commerce_cardonfile') && !empty($payment_method['settings']['cardonfile']) &&
    !empty($pane_values['bank account']['cardonfile_store']) && $pane_values['bank account']['cardonfile_store']) {
    // Build a payment details array for the credit card.
    $payment_details = array(
      'accountNumber' => $pane_values['bank account']['acct_num'],
      'routingNumber' => $pane_values['bank account']['aba_code'],
      'nameOnAccount' => $pane_values['bank account']['acct_name'],
    );

    $card_type = 'bank';
    if (!empty($pane_values['bank account']['type'])) {
      $card_type = $pane_values['bank account']['type'];
    }

    // Submit a CIM request to create the Customer Profile.
    if ($response = commerce_authnet_echeck_cim_create_customer_profile_request($payment_method, $order, $payment_details)) {
      // If the Customer Profile creation was a success, store the new card on
      // file data locally.
      if ((string) $response->messages->resultCode == 'Ok') {
        // Build a remote ID that includes the Customer Profile ID and the
        // Payment Profile ID.
        $remote_id = (string) $response->customerProfileId . '|' . (string) $response->customerPaymentProfileIdList->numericString;

        // Load the card on file values for this donation
        $expires = array();
        // If no expiration date given, use current date + 2 years
        $expire_time = strtotime("now + 2 years");

        $expires['month'] = date('m', $expire_time);
        $expires['year'] = date('Y', $expire_time);

        $card_data = commerce_cardonfile_new();
        $card_data->uid = $order->uid;
        $card_data->payment_method = $payment_method['method_id'];
        $card_data->instance_id = $payment_method['instance_id'];
        $card_data->remote_id = $remote_id;
        $card_data->card_type = !empty($card_type) ? $card_type : 'card';
        $card_data->card_name = !empty($billing_address['name_line']) ? $billing_address['name_line'] : !empty($payment_details['nameOnAccount']) ? $payment_details['nameOnAccount'] : '';
        $card_data->card_number = substr($pane_values['bank account']['acct_num'], -3);
        $card_data->card_exp_month = $expires['month'];
        $card_data->card_exp_year = $expires['year'];
        $card_data->status = 1;

        // Save and log the creation of the new card on file.
        $card = commerce_cardonfile_save($card_data);
        watchdog('commerce_authnet_echeck', 'CIM Customer Profile @profile_id created and saved to user @uid.', array('@profile_id' => (string) $response->customerProfileId, '@uid' => $order->uid));
        return $card;
      }
      elseif ((string) $response->messages->message->code == 'E00039') {
        // But if a Customer Profile already existed for this user, attempt
        // instead to add this card as a new Payment Profile to it.
        $result = array_filter(explode(' ', (string) $response->messages->message->text), 'is_numeric');
        $add_to_profile = reset($result);
      }
    }
  }

}

/**
 * Imitates the checkout form submission callback for the AIM payment method.
 */
function commerce_authnet_echeck_cim_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {
  // First attempt to load the card on file.
  $card_data = commerce_cardonfile_load($pane_values['cardonfile']);

  // Fail now if it is no longer available or the card is inactive.
  if (empty($card_data) || $card_data->status == 0) {
    drupal_set_message(t('The requested card on file is no longer valid.'), 'error');
    return FALSE;
  }

  return commerce_authnet_echeck_cardonfile_charge($payment_method, $card_data, $order, $charge);
}

/**
 * Replicates the order details population in commerce_authnet.
 */
function commerce_authnet_echeck_request_order_details(&$nvp, $order) {
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

  // Build a description for the order.
  $description = array();
  // Descriptions come from products, though not all environments have them.
  if (function_exists('commerce_product_line_item_types')) {
    foreach ($order_wrapper->commerce_line_items as $line_item_wrapper) {
      if (in_array($line_item_wrapper->type->value(), commerce_product_line_item_types())) {
        $description[] = round($line_item_wrapper->quantity->value(), 2) . 'x '
          . $line_item_wrapper->line_item_label->value();
      }
    }
  }

  // Add additional transaction invormation to the request array.
  $nvp += array(
    // Order Information.
    'x_invoice_num' => $order->order_number,
    'x_description' => substr(implode(', ', $description), 0, 255),

    // Customer Information.
    'x_email' => substr($order->mail, 0, 255),
    'x_cust_id' => substr($order->uid, 0, 20),
    'x_customer_ip' => substr(ip_address(), 0, 15),
  );

  // Prepare the billing address for use in the request.
  if (isset($order->commerce_customer_billing) && $order_wrapper->commerce_customer_billing->value()) {
    $billing_address = $order_wrapper->commerce_customer_billing->commerce_customer_address->value();

    if (empty($billing_address['first_name'])) {
      $name_parts = explode(' ', $billing_address['name_line']);
      $billing_address['first_name'] = array_shift($name_parts);
      $billing_address['last_name'] = implode(' ', $name_parts);
    }

    $nvp += array(
      // Customer Billing Address.
      'x_first_name' => substr($billing_address['first_name'], 0, 50),
      'x_last_name' => substr($billing_address['last_name'], 0, 50),
      'x_company' => substr($billing_address['organisation_name'], 0, 50),
      'x_address' => substr($billing_address['thoroughfare'], 0, 60),
      'x_city' => substr($billing_address['locality'], 0, 40),
      'x_state' => substr($billing_address['administrative_area'], 0, 40),
      'x_zip' => substr($billing_address['postal_code'], 0, 20),
      'x_country' => $billing_address['country'],
    );
  }

  // Prepare the shipping address for use in the request.
  if (isset($order->commerce_customer_shipping) && $order_wrapper->commerce_customer_shipping->value()) {
    $shipping_address = $order_wrapper->commerce_customer_shipping->commerce_customer_address->value();

    if (empty($shipping_address['first_name'])) {
      $name_parts = explode(' ', $shipping_address['name_line']);
      $shipping_address['first_name'] = array_shift($name_parts);
      $shipping_address['last_name'] = implode(' ', $name_parts);
    }

    $nvp += array(
      // Customer shipping Address.
      'x_ship_to_first_name' => substr($shipping_address['first_name'], 0, 50),
      'x_ship_to_last_name' => substr($shipping_address['last_name'], 0, 50),
      'x_ship_to_company' => substr($shipping_address['organisation_name'], 0, 50),
      'x_ship_to_address' => substr($shipping_address['thoroughfare'], 0, 60),
      'x_ship_to_city' => substr($shipping_address['locality'], 0, 40),
      'x_ship_to_state' => substr($shipping_address['administrative_area'], 0, 40),
      'x_ship_to_zip' => substr($shipping_address['postal_code'], 0, 20),
      'x_ship_to_country' => $shipping_address['country'],
    );
  }
}

/**
 * Validates an eCheck aba number.
 *
 * @param int $number
 *   The eCheck number to validate.
 *
 * @return bool
 *   TRUE or FALSE indicating the number's validity.
 *
 * @see http://www.zend.com//code/codex.php?ozid=968&single=1
 */
function commerce_authnet_echeck_payment_validate_aba($number) {
  // First, check for 9 digits and non-numeric characters.
  if (preg_match("/^[0-9]{9}$/", $number)) {
    $n = 0;
    for ($i = 0; $i < 9; $i += 3) {
      $n += (substr($number, $i, 1) * 3)
          + (substr($number, $i + 1, 1) * 7)
          + (substr($number, $i + 2, 1));
    }

    // If the resulting sum is an even multiple of ten (but not zero),
    // the aba routing number is good.
    if ($n != 0 && $n % 10 == 0) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
 * Validates an eCheck account number.
 *
 * @param int $number
 *   The eCheck account number to validate.
 *
 * @return bool
 *   TRUE or FALSE indicating the number's validity.
 */
function commerce_authnet_echeck_payment_validate_account($number) {
  // First, check for up to 20 digits and numeric characters.
  return preg_match("/^[0-9]{1,20}$/", $number);
}

/**
 * Returns an associative array of eCheck types.
 */
function commerce_authnet_echeck_payment_types() {
  return array(
    'checking' => t('Checking'),
    'savings' => t('Savings'),
  );
}
