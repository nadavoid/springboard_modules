<?php

/**
 * @file
 * Commerce based hook for commerce_paypal.
 */

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_paypal_wps_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    'payment_method' => array('paypal'),
    'offsite_processing' => array('paypal'),
    'allow_refund' => array('paypal'),
    'form callback' => 'commerce_paypal_wps_fundraiser_commerce_form',
    'redirect callback' => 'commerce_paypal_wps_fundraiser_commerce_redirect',
    'scrub callback' => 'commerce_paypal_wps_fundraiser_commerce_scrub',
    // Validation callback is commerce_paypal_wps_fundraiser_commerce_redirect_validate
    // Submit callback is commerce_paypal_wps_fundraiser_commerce_redirect_submit
    'refund callback' => 'commerce_paypal_wps_fundraiser_commerce_refund',
  );
}

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_paypal_wps_fundraiser_donation_form_config_check($node) {
  foreach ($node->gateways as $method => $gateway) {
    if (!empty($gateway['id'])  && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);
      if ($gateway_config['gateway_details']['method_id'] == 'paypal_wps') {
        // Check for sandbox mode.
        if (empty($gateway_config['gateway_details']['settings']['server'])
            || ($gateway_config['gateway_details']['settings']['server'] == 'sandbox')) {
          $messages[] = t('The PayPal WPS payment gateway is currently in sandbox mode and will not process live transactions.');
        }
        // Check for recurring only donation form.
        if ($node->recurring_setting == 'always') {
          $messages[] = t('Edit your enabled payment gateways. The PayPal WPS payment gateway is not supported on donation forms that offer only recurring payments.');
        }
      }
    }
  }

  return (!empty($messages)) ? $messages : NULL;
}

function commerce_paypal_wps_fundraiser_gateway_status_cron_check($details) {
  if ($details['method_id'] == 'paypal_wps') { 
    if($details['settings']['server'] == 'sandbox') {
      return 'sandbox';
    }
    else {
      return 'live';
    }
  }
}


/**
 * Callback function, use this form for the given gateway.
 */
function commerce_paypal_wps_fundraiser_commerce_form($payment_method, $config) {
  $method_instance = commerce_payment_method_instance_load($config['id']);
  $form = array();

  if (!empty($method_instance['settings']['show_payment_instructions'])) {
    $form['paypal_wps_information'] = array(
      '#markup' => '<span class="commerce-paypal-wps-info">' . t('After submitting the form you will be taken to PayPal to enter your payment details.') . '</span>',
    );
  }

  return $form;
}

/**
 * Callback function, handle the redirect form.
 */
function commerce_paypal_wps_fundraiser_commerce_redirect($method_instance, $donation, $order, $settings) {
  $form = drupal_get_form('commerce_paypal_wps_order_form', $order, $method_instance['settings'] + $settings);

  // Pass these fields to prepopulate the paypal fields: https://www.paypal.com/cgi-bin/webscr?cmd=_pdn_xclick_prepopulate_outside
  $paypal_fields = array(
    'mail' => 'email',
    'first_name',
    'last_name',
    'address' => 'address1',
    'address_line_2' => 'address2',
    'city',
    'state',
    'zip',
  );
  // Pull the data from the donation object
  foreach ($paypal_fields as $key => $value) {
    // If the key is a string use it as the donation array key
    $donor_key = is_string($key) ? $key : $value;
    if (!empty($donation->donation[$donor_key])) {
      $form[$value] = array(
        '#type' => 'hidden',
        '#value' => $donation->donation[$donor_key],
      );
    }
  }
  // Include the BN code if it has been set:
  if (isset($donation->gateway['gateway_details']['settings']['bn_code'])) {  
    $form['bn'] = array(
      '#type' => 'hidden',
      '#value' => $donation->gateway['gateway_details']['settings']['bn_code'],
    );
  } 
  // Set the cancel_return as form url.
  $form['cancel_return'] = array(
    '#type' => 'hidden',
    '#value' => $donation->form_url,
  );
  // Set the 'return' form item for successful PayPal checkout.
  global $base_url;
  $form['return'] = array(
    '#type' => 'hidden',
    '#value' => $base_url . '/node/' . $donation->nid . '/done?sid=' . $donation->sid,
  );
  return $form;
}

/**
 * Callback function, validate after redirect.
 */
function commerce_paypal_wps_fundraiser_commerce_redirect_validate($method_instance, $donation, $order) {
  $result = commerce_paypal_wps_redirect_form_validate($order, $method_instance);
  $success = FALSE;
  if ($result !== FALSE) {
    $success = TRUE;
  }
  return $success;
}

/**
 * Callback function, scrub the data before saving.
 */
function commerce_paypal_wps_fundraiser_commerce_scrub($submission_fields) {
  // Nothing to store for paypal
  return array();
}

/**
 * Callback function, submit after redirect.
 */
function commerce_paypal_wps_fundraiser_commerce_redirect_submit($method_instance, $donation, $order) {
  // IMPORTANT: Transaction handling for Paypal is handled on a differnt URL, when Paypal returns the IPN tracsaction ID.
  // Therefore no additional transaction work occurs here.
  return TRUE;
}

/**
 * Callback function, refund the gateway.
 */
function commerce_paypal_wps_fundraiser_commerce_refund($payment_method, $refund) {
  // Bypass capture request to PayPal if this is triggered by fundraiser_commerce_commerce_paypal_ipn_process().
  if (!empty($refund->ipn) && $refund->ipn == TRUE) {
    return TRUE;
  }
  $order = commerce_order_load($refund->did);
  $amount =  $refund->amount;
  $default_amount = $refund->donation->amount;

  // Because the IPN request sent in synchronous to this form needs to load the
  // order at the same time we are obliged to release the lock to avoid a
  // timeout error.
  entity_get_controller('commerce_order')->resetCache(array($order->order_id));

  $nvp = array(
    'METHOD' => 'RefundTransaction',
    'TRANSACTIONID' => $refund->donation->data['remote_id'],
    'AMT' => $amount,
    'REFUNDTYPE' => $default_amount - $amount ? 'Partial' : 'Full',
    'MSGSUBID' => $refund->rid,
  );

  // Submit the capture request request to PayPal.
  $response = commerce_paypal_api_request($payment_method, $nvp, $order);
  if ($response['ACK'] == "Success") {
    $refund->txn_id = $response['REFUNDTRANSACTIONID'];
    fundraiser_refund_update($refund, FALSE);
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Alter the action form settings field.
 *
 * This function was formerly an implemention of
 * hook_form_rules_ui_edit_element_alter() but behaved inconsistently here
 * in the gateway.inc file. Now it is renamed and called dynamically from
 * fundraiser_commerce_form_rules_ui_edit_element_alter() using
 * form_load_include().
 *
 * @param $form
 * @param $form_state
 */
function commerce_paypal_wps_form_rules_ui_edit_element_builder(&$form, &$form_state) {
  // Adds needed refund field elements to PayPal settings form.
  // The following two "if" checks are probably not needed, but retained for
  // readability purposes.
  if (isset($form['parameter']['payment_method']['settings']['payment_method']['method_id']['#value'])) {
    $id = $form['parameter']['payment_method']['settings']['payment_method']['method_id']['#value'];
    $is_com = isset($form['parameter']['commerce_order']) ? 1 : 0;
    if ($id == 'paypal_wps' && $is_com) {
      // Check if we're in the commerce_ui or rules UI admin pages.
      if (arg(4) == 'reaction' && arg(5) == 'manage') {
        // We're in the rules UI.
        $rule = rules_config_load(arg(6));
      }
      elseif (arg(3) == 'payment-methods' && arg(4) == 'manage') {
        // We're in the commerce payment method UI.
        $rule = rules_config_load(arg(5));
      }

      // @ToDo: is there a better way to get the rule name?
      $actions = !empty($rule) ? $rule->actions() : FALSE;
      if (!empty($actions)) {
        // Find the settings for the Authorize.net gateway action.
        foreach ($actions as $action) {
          if (arg(7) == 'edit') {
            // We're in the rules UI.
            $action_id = arg(8);
          }
          else {
            // We're in the commerce payment method UI.
            $action_id = arg(7);
          }
          if (isset($action->settings['payment_method']['method_id']) && $action->settings['payment_method']['method_id'] == 'paypal_wps' && $action->elementId() == $action_id) {
            // @TODO: is there a better way to get the action ID?
            $settings[] = $action->settings;
          }
        }
        $config = isset($settings[0]['payment_method']['settings']) ? $settings[0]['payment_method']['settings'] : '';

        $form['parameter']['payment_method']['settings']['payment_method']['settings']['api_username'] = array(
          '#type' => 'textfield',
          '#title' => t('API username'),
          '#description' => t('These credentials are only needed for initiating PayPal WPS refunds through Springboard.'),
          '#default_value' => !empty($config['api_username']) ? $config['api_username'] : '',
        );
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['api_password'] = array(
          '#type' => 'textfield',
          '#title' => t('API password'),
          '#description' => t('These credentials are only needed for initiating PayPal WPS refunds through Springboard.'),
          '#default_value' => !empty($config['api_password']) ? $config['api_password'] : '',
        );
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['api_signature'] = array(
          '#type' => 'textfield',
          '#title' => t('Signature'),
          '#description' => t('These credentials are only needed for initiating PayPal WPS refunds through Springboard.'),
          '#default_value' => !empty($config['api_signature']) ? $config['api_signature'] : '',
        );
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['bn_code'] = array(
          '#type' => 'textfield',
          '#title' => t('BN Code'),
          '#description' => t('This code should be provided by PayPal partners to be submitted along with every payment.'),
          '#default_value' => !empty($config['bn_code']) ? $config['bn_code'] : '',
        );
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['log'] = array(
          '#type' => 'checkboxes',
          '#title' => t('Log the following messages for debugging'),
          '#options' => array(
            'request' => t('API request messages'),
            'response' => t('API response messages'),
          ),
          '#default_value' => isset($config['log']) ? $config['log'] : array(
            'request' => '0',
            'response' => '0'
          ),
        );
      }
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Prevent multiple, partial donations because PayPal is returning 10007 errors after 1 or sometimes 2 partial donations sent
 * from Drupal.  Although this limitation ignores the number of refunds sent from PayPal, making this distinction could cause
 * confusion.  Note: This error is seen in the paypal sandbox; I have not tested it on production. - Josh Estep
 *
 * TODO: Fix it.  This function may be removed when this 10007 error is understood.
 */
function commerce_paypal_form__fundraiser_refund_form_alter(&$form, $form_state, $form_id) {
  $donation_id = arg(3);
  if (!is_numeric($donation_id)) {
    return;
  }
  $donation = fundraiser_donation_get_donation($donation_id);
  if (!isset($donation->refunds) || empty($donation->refunds)) {
    return;
  }

  if (!isset($donation->gateway['id']) || $donation->gateway['id'] != 'paypal_wps|commerce_payment_paypal_wps') {
    return;
  }

  // If this refund is partial than block further, partial refunds; a full refund will already have an appropriate message:
  foreach ($donation->refunds as $refund) {
    if ($refund->status == 'partially_refunded') {
      $form['refund_options'] = array(
        '#type' => 'fieldset',
        '#title' => t('Refund Options'),
        '#description' => '<strong>' . t('This donation has already been partially refunded.') . ' ' .
          t('Please make any additional refunds from your paypal.com account.') . '</strong>',
      );
      break;
    }
  }
}
