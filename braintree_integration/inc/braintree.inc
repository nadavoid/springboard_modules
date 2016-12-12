<?php
/**
 * @file
 * Collection of Braintree helper functions.
 */

/**
 * Helper function to define default settings.
 *
 * @param array $override
 *   Settings that will override the default settings.
 *
 * @return array
 *   The default settings.
 */
function braintree_default_settings($override = array()) {
  $default_settings = array(
    'environment' => 'sandbox',
    'sandbox' => array(
      'merchant_id' => '',
      'public_key' => '',
      'private_key' => '',
      'merchant_account_id' => '',
      'channel' => '',
    ),
    'production' => array(
      'merchant_id' => '',
      'public_key' => '',
      'private_key' => '',
      'merchant_account_id' => '',
      'channel' => '',
    ),
  );
  $default_settings = array_merge($default_settings, $override);
  drupal_alter('braintree_default_settings', $default_settings);
  return $default_settings;
}

/**
 * Get Braintree settings.
 *
 * @param array $settings
 *   An array of settings that will override the default settings.
 * @param bool $active
 *   The type of settings to return. TRUE will return settings for the current
 *   environment (sandbox/production) only.
 *
 * @return array
 *   The settings array.
 */
function braintree_settings($settings, $active = TRUE) {
  $cache = &drupal_static(__FUNCTION__);

  if (empty($cache)) {
    $cache = braintree_default_settings($settings);
  }

  if ($active) {
    $environment = $cache['environment'];
    $return = $cache[$environment];
    $return['environment'] = $environment;
    return $return;
  }

  return $cache;
}

/**
 * Configures the Braintree library API with our configuration settings.
 *
 * @param array $settings
 *   The settings array used to configure the Braintree library API.
 */
function braintree_initialize($settings) {
  $booted = &drupal_static(__FUNCTION__);
  if (!isset($booted)) {
    $booted = FALSE;
  }

  // If we've already initialized Braintree, return early.
  if ($booted) {
    return;
  }

  // Load the Braintree PHP library.
  $library = libraries_load('braintree_php');
  if (empty($library['loaded'])) {
    // Couldn't load the library. Try to get some info and watchdog it.
    $replacements = array(
      '@error' => '',
      '@error_message' => '',
    );
    if (!empty($library['error'])) {
      $replacements['@error'] = $library['error'] . ':';
    }
    if (!empty($library['error message'])) {
      $replacements['@error_message'] = $library['error message'];
    }
    watchdog('braintree', 'braintree_php library not found! @error @error_message', $replacements, WATCHDOG_ALERT);
    return;
  }

  // Load the settings and configure the Braintree PHP library.
  $settings = braintree_settings($settings);
  $needed_settings_loaded = !empty($settings['merchant_id']) && !empty($settings['public_key']) && !empty($settings['private_key']) && !empty($settings['environment']);
  if ($needed_settings_loaded) {
    Braintree_Configuration::merchantId($settings['merchant_id']);
    Braintree_Configuration::publicKey($settings['public_key']);
    Braintree_Configuration::privateKey($settings['private_key']);
    Braintree_Configuration::environment($settings['environment']);
    $booted = TRUE;
  }
  else {
    drupal_set_message(t('Braintree has not been configured.'), 'error');
  }
}

/**
 * Logs error message to watchdog when Braintree throws an exception.
 *
 * @param string|\Braintree\Result\Exception $exception
 *   An error message or an Braintree exception object.
 *
 * @return string
 *   The error message.
 *
 * @see https://api.drupal.org/api/drupal/includes%21bootstrap.inc/function/watchdog/7.x
 */
function braintree_report_error($exception) {
  if (empty($exception) || !is_object($exception) || !property_exists($exception, 'getMessage')) {
    return;
  }

  $message = $exception->getMessage();

  $backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
  $caller = _drupal_get_last_caller($backtrace);

  $variables = array(
    '@message' => $message,
    '%function' => $caller['function'],
    '%file' => $caller['file'],
    '%line' => $caller['line'],
    '%data' => $backtrace[1]['function'],
  );
  $admin_message = 'API exception: @message in %function (line %line of %file). Exception: @data';

  watchdog('Braintree', $admin_message, $variables, WATCHDOG_ERROR);

  return $message;
}

/**
 * Logs an error message from a Braintree failed result to watchdog.
 *
 * @param \Braintree\Result\Instance $result
 *   The result object.
 *
 * @return string
 *   The error message.
 */
function braintree_report_result($result) {
  if (empty($result) || !is_object($result) || !property_exists($result, 'message')) {
    return;
  }

  $message = t('@message Please try to fix the problem and try again.', array(
    '@message' => $result->message,
  ));

  $backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, 2);

  $variables = array(
    '@message' => $result->message,
    '@name' => $backtrace[1]['function'],
    '!data' => substr(print_r($result, TRUE), 0, 10000),
  );
  $admin_message = 'API error: @message in function @name <br><pre>!data</pre>';

  watchdog('Braintree', $admin_message, $variables, WATCHDOG_NOTICE);

  return $message;
}

/**
 * Gets the payment_method_nonce from the post variables if it exists.
 *
 * Optionally override with a test_nonce if supplied in the URL. This provides
 * an easy way to test the valid and invalid nonces provided by Braintree at
 * https://developers.braintreepayments.com/reference/general/testing/php.
 */
function braintree_get_nonce() {
  $nonce = NULL;
  if (!empty($_POST['payment_method_nonce'])) {
    $nonce = check_plain($_POST['payment_method_nonce']);
  }
  if (!empty($_GET['test_nonce'])) {
    $nonce = check_plain($_GET['test_nonce']);
  }
  return $nonce;
}

/**
 * Check if payment method is credit card.
 *
 * @param \Braintree\PayPalAccount|\Braintree\CreditCard $payment_method
 *   The payment method object.
 *
 * @return bool
 *   TRUE if this payment method is of type credit card.
 */
function braintree_payment_method_is_credit_card($payment_method) {
  $class = get_class($payment_method);
  return strpos($class, 'CreditCard') !== FALSE;
}

/**
 * Check if payment method is PayPal.
 *
 * @param \Braintree\PayPalAccount|\Braintree\CreditCard $payment_method
 *   The payment method object.
 *
 * @return bool
 *   TRUE if this payment method is of type paypal.
 */
function braintree_payment_method_is_paypal($payment_method) {
  $class = get_class($payment_method);
  return strpos($class, 'PayPalAccount') !== FALSE;
}

/**
 * Get subscription ID from an order.
 *
 * @param object $order
 *   The order object.
 *
 * @return bool|string
 *   The remote transaction ID, or FALSE.
 */
function braintree_commerce_order_subscription_id($order) {
  if (isset($order->data['braintree_subscription_id'])) {
    return $order->data['braintree_subscription_id'];
  }

  $subscription_id = braintree_commerce_payment_transaction_remote_id($order);
  return $subscription_id;
}

/**
 * Get the remote ID of payment transaction from an order object.
 *
 * When creating a Braintree transaction sale, the remote ID is the transaction
 * ID; when creating a Braintree subscription, the remote ID is the subscription
 * ID.
 *
 * @param object $order
 *   The order object.
 *
 * @return bool|string
 *   The remote transaction ID, or FALSE.
 */
function braintree_commerce_payment_transaction_remote_id($order) {
  $payments = commerce_payment_transaction_load_multiple(array(), array('order_id' => $order->order_id, 'status' => 'success'));
  if ($payments) {
    // If you just have one and that's all you want...
    $payment = array_shift($payments);
    return $payment->remote_id;
  }

  return FALSE;
}

/**
 * Helper function to determine if we're operating in sandbox mode.
 *
 * @return bool
 *   TRUE if we're in sandbox mode, FALSE otherwise.
 */
function braintree_is_sandbox($settings) {
  $settings = braintree_settings($settings, 'active');
  return $settings['environment'] == 'sandbox';
}

/**
 * Get time offset between two timezones.
 *
 * @param string $remote_tz
 *   The timezone, defaults to the Braintree local timezone.
 * @param string $origin_tz
 *   The originating timezone to calculate the offset for.
 *
 * @return bool|int
 *   The offset between the two timezones, or FALSE if failure.
 */
function braintree_get_timezone_offset($remote_tz = 'America/Chicago', $origin_tz = NULL) {
  if (empty($origin_tz)) {
    $origin_tz = date_default_timezone_get();
    if (!is_string($origin_tz)) {
      // A UTC timestamp was returned -- bail out!
      return FALSE;
    }
  }

  $origin_dtz = new DateTimeZone($origin_tz);
  $remote_dtz = new DateTimeZone($remote_tz);
  $origin_dt = new DateTime("now", $origin_dtz);
  $remote_dt = new DateTime("now", $remote_dtz);
  $offset = $origin_dtz->getOffset($origin_dt) - $remote_dtz->getOffset($remote_dt);

  return $offset;
}

/**
 * Helper function to check a transaction response for fraud.
 *
 * @param array $context
 *   An array holding the result as a child element.
 *
 * @return bool
 *   TRUE if the transaction passes, FALSE otherwise.
 */
function braintree_check_fraud(&$context) {
  $result = &$context['result'];

  if (!empty($result->transaction->riskData)) {
    $decision = $result->transaction->riskData->decision;

    // Send an alert email for Decline/Review.
    // @todo add the variable braintree_email_on_fraud_detected to the config
    // form.
    if ($decision == 'Review' && variable_get('braintree_email_on_fraud_detected', FALSE)) {
      $from = variable_get('site_mail');
      $braintree_env_settings = !empty($braintree_settings['environment']) ? $braintree_settings[$braintree_settings['environment']] : array();
      $to = $braintree_env_settings['alert_email'] ?: variable_get('site_mail');
      $params = array(
        'order' => $context['order'],
        'txn_id' => $result->transaction->id,
        'decision' => $decision,
      );
      $lang = language_default();

      if ($from && $to) {
        drupal_mail('braintree', 'transaction_fraud', $to, $lang, $params, $from);
      }
    }

    // Set form error if the decision is "Decline".
    if ($decision == 'Decline') {
      watchdog('braintree', 'Payment declined.', array(), WATCHDOG_ERROR, NULL);
      return array(FALSE, t('Sorry, your payment transaction was declined.'));
    }

    // Set success to TRUE on the $result object if the decision is "Review".
    if ($decision == 'Review') {
      $result->success = TRUE;
    }
  }

  return array(TRUE, TRUE);
}
