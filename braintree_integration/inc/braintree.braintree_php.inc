<?php
/**
 * @file
 * Functions that integrate with the Braintree PHP library.
 */

/**
 * Void a Braintree transaction.
 *
 * @param int $transaction_id
 *   The transaction ID.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   An array where the first element is a boolean indicating whether the void
 *   was successful, and the second element being an array of related
 *   information like the result object and any error messages.
 */
function braintree_transaction_void($transaction_id, &$context = array(), $settings = array()) {
  braintree_initialize($settings);
  try {
    $context['result'] = Braintree_Transaction::void($transaction_id);
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  if ($context['result']->success) {
    return array(TRUE, $context);;
  }

  $context['message'] = braintree_report_result($result);
  return array(FALSE, $context);
}

/**
 * Return a list of authorized transactions of a customer.
 *
 * @param Braintree_Customer $customer
 *   The customer object.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   An array where the first element is a boolean value indicating if the
 *   request succeeded and the second element being an array of related
 *   information.
 */
function braintree_authorized_transactions($customer, &$context = array(), $settings = array()) {
  braintree_initialize($settings);
  try {
    $context['collection'] = Braintree_Transaction::search(array(
      Braintree_TransactionSearch::customerId()->is($customer->id),
      Braintree_TransactionSearch::status()->in(array(Braintree_Transaction::AUTHORIZED)),
    ));
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  return array(TRUE, $collection);
}

/**
 * Load Braintree transaction by ID.
 *
 * @param int $transaction_id
 *   The transaction ID.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   An array where the first element is a boolean indicating whether the
 *   transaction was found, and the second element being an array containing the
 *   result and any error messages that might have occurred.
 */
function braintree_transaction_find($transaction_id, &$context = array(), $settings = array()) {
  braintree_initialize($settings);
  try {
    // Execute the API sale method to create a sale object.
    $context['transaction'] = Braintree_Transaction::find($transaction_id);
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  return array(TRUE, $context);
}

/**
 * Create a Braintree transaction sale.
 *
 * @param array $sale_data
 *   The sale data which will be sent to Braintree.
 * @param array $context
 *   An array which holds additional information.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   A result array where the first element is a boolean of whether or not the
 *   transaction was succsesful and the second element being an array of related
 *   information like the result and any error messages that might have
 *   occurred.
 */
function braintree_transaction_sale(&$sale_data, &$context = array(), $settings = array()) {
  // Allow other modules to alter the sale before sending it to Braintree.
  drupal_alter('braintree_transaction_sale_data', $sale_data, $context);

  if (empty($context['order'])) {
    return array(FALSE, $context);
  }

  braintree_initialize($settings);

  try {
    // Execute the API sale method to create a sale object.
    $context['result'] = Braintree_Transaction::sale($sale_data);
  }
  catch (Braintree_Exception_Unexpected $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  if ($context['result'] instanceof Braintree_Result_Error) {
    $context['error'] = braintree_report_error($context['result']->errors->deepAll());
    return array(FALSE, $context);
  }

  list($approved, $message) = braintree_check_fraud($context);
  if (!$approved) {
    $context['result']->status = FALSE;
    $context['error'] = $message;
    return array(FALSE, $context);
  }

  if ($context['result']->success) {
    // Invoke post hook when sale is completed successfully.
    foreach (module_implements('braintree_transaction_sale_complete') as $module) {
      $function = $module . '_braintree_transaction_sale_complete';
      $function($context);
    }
    return array(TRUE, $context);
  }

  return array(FALSE, $context);
}

/**
 * Updates a user's stored Braintree payment information.
 *
 * @param int $card_id
 *   The card ID.
 * @param array $params
 *   An array of parameters to pass to the update method.
 * @param array $settings
 *   An optional array of sttings to initialize Braintree with.
 * @param array $context
 *   An optional array that will be returned with new information.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   An array where the first element is the success condition (TRUE or FALSE),
 *   and the second element is an array of additional information like the
 *   result and any related error messages.
 */
function braintree_update_card($card_id, $params, $settings = array(), &$context = array(), $settings = array()) {
  if (!is_int($card_id)) {
    $context['error'] = t('Invalid card ID provided.');
    return array(FALSE, $context);
  }

  braintree_initialize($settings);

  try {
    $context['result'] = Braintree_PaymentMethod::update($card_id, $params);
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    return array(FALSE, $context);
  }

  return array(TRUE, $context);
}

/**
 * Deletes a customer's stored Braintree information.
 *
 * @param int $card_id
 *   The user's remote card ID.
 * @param array $context
 *   An optional array to pass that will be returned with related information
 *   about the request.
 * @param array $settings
 *   An array of settings to initialize Braintree with.
 *
 * @return array
 *   An array where the first element is a boolean indicating the result of the
 *   deletion and the second element being an array of related information like
 *   the result and any error messages.
 */
function braintree_delete_card($card_id, $settings = array()) {
  braintree_initialize($settings);
  $context = array();
  try {
    $context['result'] = Braintree_PaymentMethod::delete($card_id);
  }
  catch (Exception $e) {
    $context['error'] = braintree_report_error($e);
    watchdog('commerce_braintree_web', 'Error deleting cardonfile id: %card_id (Braintree payment_method id: %remote_id) with Braintree error message: %error_message', array(
      '%card_id' => $card_data->card_id,
      '%remote_id' => $card_data->remote_id,
      '%error_message' => $context['error'],
    ), WATCHDOG_ERROR);
    return array(FALSE, $context);
  }

  return array(TRUE, $context);
}
