<?php
/**
 * @file
 * Provides Braintree admin settings pages.
 */

/**
 * Braintree admin settings form.
 */
function braintree_admin_settings($form, &$form_state) {
  $form['moved'] = array(
    '#markup' => 'This form has been moved to the gateway config pages. Go to: Store > Configuration > Payment Methods',
  );

  return $form;
}

function braintree_admin_settings_js($form, $form_state) {
  return $form['production'];
}

/**
 * Validate handler for Braintree settings form.
 *
 * @todo implement test connection function
 */
function braintree_admin_settings_validate($form, &$form_state) {
  $vaules = $form_state['values'];

//  if ($vaules['environment'] == 'production') {
//    foreach (array_keys($form['production']) as $field) {
//      if (empty($vaules[$field])) {
//        form_set_error('production][' . $field, t('@field_name is required', array(
//          '@field_name' => $form['production'][$field]['#title']
//        )));
//      }
//    }
//  }

//  $merchantAccount = Braintree_MerchantAccount::find('the_merchant_account_id');
}

/**
 * Submit callback; test braintree connection.
 */
function braintree_test_connection_submit($form, &$form_state) {
  form_state_values_clean($form_state);
  $values = $form_state['values'];

  $environment = $values['environment'];
  $settings = $values[$environment];
  $settings['environment'] = $environment;

  braintree_initialize($settings);

  try {
    $merchantAccount = Braintree_MerchantAccount::find($settings['merchant_account_id']);
    drupal_set_message(t('Braintree connection succeeded.'));
  }
  catch (Exception $e) {
    $error_message = braintree_report_error($e, TRUE);
    $message = t('Braintree connection failed: @message. Please check Braintree gateway credentials.', array(
      '@message' => $error_message,
    ));

    drupal_set_message($message, 'error');
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for Braintree settings form.
 */
function braintree_admin_settings_submit($form, &$form_state) {
  form_state_values_clean($form_state);
  $values = $form_state['values'];

  variable_set('braintree_debug', $values['braintree_debug']);
  unset($values['braintree_debug']);

  variable_set('braintree_settings', $values);
}
