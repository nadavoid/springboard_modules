<?php
/**
 * @file
 * Functions that integrate with the Braintree PHP library.
 */

/**
 * Void a Braintree transaction.
 *
 * @param $transaction_id
 * @param string $error_message
 * @return bool|object
 */
function braintree_transaction_void($transaction_id, &$error_message = '') {
  try {
    $result = Braintree_Transaction::void($transaction_id);
  }
  catch (Exception $e) {
    braintree_report_error($e);
    return FALSE;
  }

  if ($result->success) {
    return $result;
  }

  braintree_report_result($result);
  return FALSE;
}

/**
 * Return a list of authorized transactions of a customer.
 *
 * @param $customer
 * @return mixed
 */
function braintree_authorized_transactions($customer) {
  try {
    $collection = Braintree_Transaction::search(array(
      Braintree_TransactionSearch::customerId()->is($customer->id),
      Braintree_TransactionSearch::status()->in(array(Braintree_Transaction::AUTHORIZED)),
    ));
  }
  catch (Exception $e) {
    braintree_report_error($e);
    return FALSE;
  }

  return $collection;
}

/**
 * Load Braitnree transaction by ID.
 *
 * @param $transaction_id
 * @return bool
 */
function braintree_transaction_find($transaction_id) {
  try {
    // Execute the API sale method to create a sale object.
    $transaction = Braintree_Transaction::find($transaction_id);
  }
  catch (Exception $e) {
    braintree_report_error($e);
    return FALSE;
  }

  return $transaction;
}

/**
 * Create a Braintree transaction sale.
 *
 * @param array $sale_data
 *   The sale data which will be sent to Braintree.
 * @param array $context
 *   An array which holds additional information.
 *
 * @return array|bool
 */
function braintree_transaction_sale(&$sale_data, &$context) {
  // Allow other modules to alter the sale before sending it to Braintree.
  drupal_alter('braintree_transaction_sale_data', $sale_data, $context);

  if (empty($context['order'])) {
    return array(FALSE, FALSE);
  }

  try {
    $order = $context['order'];
    $payment_method = explode('|', $order->data['fundraiser_commerce']);
    $payment_method = reset($payment_method);

    // Execute the API sale method to create a sale object.
    $result = Braintree_Transaction::sale($sale_data);
    $context['result'] = $result;
  }
  catch (Exception $e) {
    $error_message = braintree_report_error($e);
    return array(FALSE, $error_message);
  }

  list($approved, $message) = braintree_check_fraud($context);
  if (!$approved) {
    return array(FALSE, $message);
  }

  if ($context['result']->success) {
    // Invoke post hook when sale is completed successfully.
    foreach (module_implements('braintree_transaction_sale_complete') as $module) {
      $function = $module . '_braintree_transaction_sale_complete';
      $function($result, $context);
    }
    return array(TRUE, $context['result']);
  }

  return array(FALSE, $context['result']);
}

/**
 * Updates a user's stored Braintree payment information.
 *
 * @param int $card_id
 *   The card ID.
 * @param array $params
 *   An array of parameters to pass to the update method.
 *
 * @return array
 *   An array where the first element is the success condition (TRUE or FALSE),
 *   and the second element is either an error message (if the success condition
 *   was FALSE) or the \Braintree\Result object (if the success condition was
 *   TRUE).
 */
function braintree_update_card($card_id, $params) {
  if (!is_int($card_id)) {
    return array(FALSE, t('Invalid card ID provided.'));
  }

  try {
    $result = Braintree_PaymentMethod::update($card, $params);
  }
  catch (Exception $e) {
    $error_message = braintree_report_error($e);
    return array(FALSE, $e);
  }

  return array(TRUE, $result);
}

/**
 * Deletes a customer's stored Braintree information.
 */
function braintree_delete_card($card_id) {
  try {
    $result = Braintree_PaymentMethod::delete($card_id);
  }
  catch (Exception $e) {
    $error_message = braintree_report_error($e);
    watchdog('commerce_braintree_web', 'Error deleting cardonfile id: %card_id (Braintree payment_method id: %remote_id) with Braintree error message: %error_message', array(
      '%card_id' => $card_data->card_id,
      '%remote_id' => $card_data->remote_id,
      '%error_message' => $error_message,
    ), WATCHDOG_ERROR);
    return FALSE;
  }
}
