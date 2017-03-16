<?php
/**
 * @file
 * Holds functions that utilize the Braintree PHP library.
 */

/**
 * Look up Braintree customer object by customer ID.
 *
 * @param int $customer_id
 *   The customer ID.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return bool|\Braintree\Customer
 *   The Braintree\Customer object or FALSE if not found.
 */
function braintree_vault_customer_load($customer_id, $settings = array()) {
  braintree_initialize($settings);
  try {
    $customer = Braintree_Customer::find($customer_id);
    $context['result']->result = $customer;
    return array(TRUE, $customer);
  }
  catch (Braintree_Exception_NotFound $e) {
    $error_message = braintree_report_error($e);
    $context['result']->message = $error_message;
    return array(FALSE, $context['result']);
  }
}

/**
 * Save customer record to Braintree vault.
 *
 * @param array $customer_data
 *   The customer data used to create the customer.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return bool|\Braintree_Customer
 *   The Braintree_Customer object or FALSE if the customer wasn't saved.
 */
function braintree_vault_customer_save($customer_data, $settings = array()) {
  drupal_alter('braintree_vault_customer_data', $customer_data);

  braintree_initialize($settings);

  try {
    $result = Braintree_Customer::create($customer_data);
  }
  catch (Exception $e) {
    braintree_report_error($e);
    return FALSE;
  }

  if ($result->success) {
    // Invoke post hook when sale is completed successfully.
    foreach (module_implements('braintree_vault_customer_saved') as $module) {
      $function = $module . '_braintree_vault_customer_saved';
      $function($result, $context);
    }

    return $result;
  }

  braintree_report_error($result);
  return FALSE;
}

/**
 * Save payment method to Braintree vault.
 *
 * @param array $payment_method
 *   The payment method to create or update.
 * @param array $context
 *   An array of contextual information for other code.
 * @param array $settings
 *   An optional array of settings to initialize the braintree library with.
 *
 * @return array
 *   An array structure where the first element is whether the payment method
 *   was saved successfully, and the second element is a contextual array of
 *   related information like error messages.
 */
function braintree_vault_payment_method_save($payment_method, $context = array(), $settings = array()) {
  drupal_alter('braintree_payment_method', $payment_method, $context);

  braintree_initialize($settings);

  try {
    if ($context['op'] == 'edit') {
      $context['result'] = Braintree_PaymentMethod::update($payment_method['paymentMethodToken'], $context['options']);
    }
    else {
      $context['result'] = Braintree_PaymentMethod::create($payment_method);
    }
  }
  catch (Braintree_Exception_Unexpected $e) {
    $error_message = braintree_report_error($e);
    $context['error'] = $error_message;
    return array(FALSE, $context);
  }

  if (!empty($context['result']->success)) {
    // When adding a card, check if it is a duplicate payment method.
    // Currently, the only way to achieve this is by adding the card first and
    // determining if the card's unique idenifier ID exists for the user.
    // @todo Re-implement this when/if Braintree ever supports looking up a
    // user's card before it has to be saved.
    if ($context['op'] == 'add' && !empty($context['customer']['creditCards'] && in_array($context['result']->paymentMethod->uniqueNumberIdentifier, $context['customer']['creditCards']))) {
      // If we already have this card stored, delete this card and update the
      // stored card.
      try {
        Braintree_PaymentMethod::delete($context['result']->paymentMethod->token);
        $token = db_query('SELECT payment_method_token FROM {braintree_vault_payment_methods} WHERE unique_identifier = :u AND uid = :uid', array(
          ':u' => $context['result']->paymentMethod->uniqueNumberIdentifier,
          ':uid' => $context['customer']['customer']->uid,
        ))->fetchField();
        unset($payment_method['paymentMethodNonce']);
        unset($payment_method['customerId']);
        $payment_method['expirationMonth'] = $context['result']->paymentMethod->expirationMonth;
        $payment_method['expirationYear'] = $context['result']->paymentMethod->expirationYear;
        $payment_method['options']['verifyCard'] = FALSE;
        $context['result'] = Braintree_PaymentMethod::update($token, $payment_method);
        $context['result']->token = $token;
      }
      catch (Braintree_Result_Error $e) {
        $error_message = braintree_report_error($e);
        $context['error'] = $error_message;
        return array(FALSE, $context);
      }
    }

    // @todo Determine if this should be the current user or the user specified
    // in the parameters.
    global $user;
    braintree_vault_customer_cache_reset($user->uid);

    // Invoke post hook when payment method is added or updated successfully.
    foreach (module_implements('braintree_vault_payment_method_saved') as $module) {
      $function = $module . '_braintree_vault_payment_method_saved';
      $function($context);
    }

    return array(TRUE, $context);
  }

  $error_message = braintree_report_result($context['result']);
  return array(FALSE, $context);
}

/**
 * Delete payment method from Braintree vault.
 *
 * @param array $payment_method
 *   The payment method to delete.
 * @param array $context
 *   An array of contextual information for other code.
 * @param array $settings
 *   An optional array of settings to initialize the braintree library with.
 *
 * @return array
 *   An array structure where the first element is whether the payment method
 *   was saved successfully, and the second element is a contextual array of
 *   related information like error messages.
 */
function braintree_vault_payment_method_delete($payment_method, $context = array(), $settings = array()) {
  braintree_initialize($settings);

  try {
    $context['result'] = Braintree_PaymentMethod::delete($payment_method->token);
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  if ($context['result']->success) {
    global $user;
    braintree_vault_customer_cache_reset($user->uid);

    // Invoke post hook when sale is completed successfully.
    foreach (module_implements('braintree_vault_payment_method_delete') as $module) {
      $function = $module . '_braintree_vault_payment_method_delete';
      $function($context);
    }

    return array(TRUE, $context);
  }

  $context['error'] = braintree_report_error($context['result']);
  return array(FALSE, $context);
}

/**
 * Change default payment method from Brantree vault.
 *
 * @param \Braintree\PayPalAccount|\Braintree\CreditCard $payment_method
 *   An object of Braintree payment method.
 * @param array $context
 *   An array of contextual information for other code.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   An array structure where the first element is whether the payment method
 *   was saved successfully, and the second element is a contextual array of
 *   related information like error messages.
 */
function braintree_vault_payment_method_make_default($payment_method, $context = array(), $settings = array()) {
  if (braintree_payment_method_is_paypal($payment_method)) {
    $class = 'Braintree_PayPalAccount';
  }
  else {
    $class = 'Braintree_PaymentMethod';
  }

  braintree_initialize($settings);

  try {
    $context['result'] = $class::update($payment_method->token, array(
      'options' => array('makeDefault' => TRUE),
    ));
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  if ($context['result']->success) {
    // Invoke post hook when sale is completed successfully.
    foreach (module_implements('braintree_vault_payment_method_make_default') as $module) {
      $function = $module . '_braintree_vault_payment_method_make_default';
      $function($context);
    }

    return array(TRUE, $context);
  }

  $context['error'] = braintree_report_error($context['result']);
  return array(FALSE, $context);
}

/**
 * Update customer record with Braintree vault.
 *
 * @param int $customer_id
 *   The customer's Braintree ID.
 * @param array $customer_data
 *   The customer data used to update the customer.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return bool|\Braintree_Customer
 */
function braintree_vault_customer_update($customer_id, $customer_data, $settings = array()) {
  drupal_alter('braintree_vault_customer_data', $customer_data);

  braintree_initialize($settings);

  try {
    $result = Braintree_Customer::update($customer_id, $customer_data);
  }
  catch (Exception $e) {
    braintree_report_error($e);
    return FALSE;
  }

  if ($result->success) {
    // Invoke post hook when sale is completed successfully.
    foreach (module_implements('braintree_vault_customer_updated') as $module) {
      $function = $module . '_braintree_vault_customer_updated';
      $function($result, $context);
    }

    return $result;
  }

  braintree_report_error($result);
  return FALSE;
}

/**
 * Update a customer's address in Braintree Vault.
 *
 * @param int $customer_id
 *   The customer ID.
 * @param string $address_id
 *   The address ID.
 * @param array $data
 *   The new address data.
 * @param array $settings
 *   The payment gateway settings to initialize Braintree with.
 *
 * @return bool
 *   TRUE if updated successfully, FALSE otherwise.
 */
function braintree_vault_address_update($customer_id, $address_id, $data, $settings = array()) {
  braintree_initialize($settings);

  drupal_alter('braintree_vault_customer_address_update', $customer_id, $address_id, $data);

  try {
    $result = Braintree_Address::update($customer_id, $address_id, $data);
  }
  catch (Exception $e) {
    $message = braintree_report_error($e);
    return array(FALSE, $message);
  }

  return array(TRUE, $result);
}
