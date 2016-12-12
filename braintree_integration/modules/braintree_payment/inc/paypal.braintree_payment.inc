<?php
/**
 * @file
 * Holds code for paypal fields on Braintree payment form.
 */

/**
 * Implements hook_braintree_payment_form__PAYMENT_TYPE__alter().
 */
function braintree_payment_braintree_payment_form__paypal__alter(&$context) {
  $payment_method_instance = $context['payment_method_instance'];
  if ($payment_method_instance['method_id'] != 'braintree_paypal') {
    return;
  }

  $form = &$context['form'];
  $form_state = &$context['form_state'];
  $js_settings = &$context['js_settings'];

  $form['braintree']['new']['paypal'] = array(
    '#type' => 'container',
    '#id' => 'paypal-container',
  );
  $form['braintree']['new']['paypal']['nonce'] = array(
    '#type' => 'hidden',
    '#name' => 'payment_method_nonce',
  );
  $form['braintree']['new']['paypal']['loggedin'] = array(
    '#type' => 'container',
    '#id' => 'braintree-paypal-loggedin',
  );
  $form['braintree']['new']['paypal']['loggedin']['contents'] = array(
    '#markup' => '<span id="bt-pp-name">PayPal</span><span id="bt-pp-email"></span><button id="bt-pp-cancel">Cancel</button>',
  );

  $js_settings['integration'] = 'paypal';
  $js_settings['paypalContainer'] = 'paypal-container';
  $js_settings['enabledMethods'][] = 'paypal';
  $js_settings['paypal']['autofill'] = $payment_method_instance['settings']['autofill'];
}
