<?php

/**
 * @file
 * Commerce based hook for commerce_authnet.
 */

/**
 * Implements hook_fundraiser_commerce_fundraiser_gateway_info().
 */
function commerce_authnet_fundraiser_commerce_fundraiser_gateway_info() {
  return array(
    'payment_method' => array('credit'),
    'allow_recurring' => array('credit'),
    'allow_refund' => array('credit'),
    'form callback' => 'commerce_authnet_fundraiser_commerce_form',
    'cleanup callback' => 'commerce_authnet_fundraiser_commerce_cleanup',
    'scrub callback' => 'commerce_authnet_fundraiser_commerce_scrub',
    'expire callback' => 'commerce_authnet_fundraiser_commerce_expire',
    'validate callback' => 'commerce_authnet_fundraiser_commerce_validate',
    'charge callback' => 'commerce_authnet_fundraiser_commerce_charge',
    'update callback' => 'commerce_authnet_fundraiser_commerce_update',
    'refund callback' => 'commerce_authnet_fundraiser_commerce_refund',
    'cardonfile callback' => 'commerce_authnet_fundraiser_commerce_cardonfile',
    'response fields callback' => 'commerce_authnet_fundraiser_commerce_response_fields',
    'response save callback' => 'commerce_authnet_fundraiser_commerce_response_save',
  );
}

/**
 * Implements hook_fundraiser_donation_form_config_check().
 */
function commerce_authnet_fundraiser_donation_form_config_check($node) {
  foreach ($node->gateways as $method => $gateway) {
    if (!empty($gateway['id']) && !empty($gateway['status'])) {
      $gateway_config = _fundraiser_gateway_info($gateway['id']);
      if ($gateway_config['gateway_details']['method_id'] == 'authnet_aim' &&
        $gateway_config['gateway_details']['settings']['txn_mode'] != AUTHNET_TXN_MODE_LIVE) {
        return array(t('The authorize.net payment gateway is currently in test mode and will not process live transactions.'));
      }
    }
  }
}

function commerce_authnet_fundraiser_gateway_status_cron_check($details) {
  if ($details['method_id'] == 'authnet_aim') {
    if ($details['settings']['txn_mode'] != AUTHNET_TXN_MODE_LIVE) {
      return $details['settings']['txn_mode'];
    }
    else {
      return 'live';
    }
  }
}


/**
 * Callback function, use this form for the given gateway.
 */
function commerce_authnet_fundraiser_commerce_form($payment_method) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_form();
}

/**
 * Callback function, clean up the submission results after submission before they are returned to fundraiser.
 */
function commerce_authnet_fundraiser_commerce_cleanup($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_cleanup($submission_fields);
}

/**
 * Callback function, scrub the data before saving.
 */
function commerce_authnet_fundraiser_commerce_scrub($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_scrub($submission_fields);
}

/**
 * Callback function, return the user entered experation dates for this submission.
 */
function commerce_authnet_fundraiser_commerce_expire($submission_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  return _fundraiser_commerce_credit_card_expire($submission_fields);
}

/**
 * Validate the submitted values with the commerce validate function
 */
function commerce_authnet_fundraiser_commerce_validate($submission_fields, $payment_fields) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $form_parents = array_merge($payment_fields['#parents'], array('credit'));
  _fundraiser_commerce_credit_card_validate($submission_fields, $form_parents);
  return $submission_fields;
}

/**
 * Callback function, charge gateway.
 */
function commerce_authnet_fundraiser_commerce_charge($method_instance, $donation) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  // Translate from donation settings to the correct values for the plugin.

  $order = commerce_order_load($donation->did);
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $wrapper->commerce_order_total->value();

  $pane_form = array(); // Not actually used anywhere in this system, so ignore it.

  $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);

  // Pad single digit exp months for authnet validation
  $pane_values['credit_card']['exp_month'] = sprintf("%02d", $pane_values['credit_card']['exp_month']);

  // Add fundraiser commerce data to the pane values array
  _fundraiser_commerce_submit_handler_pane_values($pane_values, $donation);

  // Execute the authnet submit handler
  $result = commerce_authnet_aim_submit_form_submit($method_instance, $pane_form, $pane_values, $order, $charge);
  $success = FALSE;
  if ($result !== FALSE) {
    $success = TRUE;
  }

  // Perform post processing functions
  _fundraiser_commerce_charge_submit_form_process($success, $method_instance, $pane_values, $donation);

  return $success;
}

/**
 * Card on file callback
 *
 * We use a custom callback since we may need to update an existing CIM profile
 */
function commerce_authnet_fundraiser_commerce_cardonfile($donation) {
  // Use standard function to setup the card on file fields
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $cardonfile_fields = _fundraiser_commerce_credit_card_cardonfile_fields($donation);

  // Authorize.net does not check the exp date when checking for duplicate profiles
  // So we need to search locally to see if we have an existing profile for a card number
  // Only perform this when we're in an active non-reference charge
  if (isset($donation->reference_charge) && $donation->reference_charge === FALSE) {
    // Query the db for an existing record
    $card_data = _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields);

    // If we have a profile for this user update it in the CIM
    if ($card_data) {
      // Make the update request to authorize.net
      commerce_authnet_fundraiser_commerce_update($donation, $card_data);
    }
  }

  return $cardonfile_fields;
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
function commerce_authnet_fundraiser_commerce_update($donation, $card_data = NULL) {
  module_load_include('inc', 'fundraiser_commerce', 'includes/fundraiser_commerce.credit_card');
  $payment_method = $donation->gateway['gateway_details'];

  // If no card data was passed load the profile from the donation if its set
  if (empty($card_data) && !empty($donation->data['cardonfile'])) {
    $card_data = commerce_cardonfile_load($donation->data['cardonfile']);
  }

  if (empty($card_data)) {
    // Log the missing card data
    watchdog(
      'fundraiser',
      'A card update was attempted on donation @id but no stored card data was found.',
      array('@id' => $donation->did),
      WATCHDOG_CRITICAL
    );

    return $card_data;
  }

  // Pull the new credit card values from the donation
  $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);

  // Load the card on file values for this donation
  $cardonfile_fields = _fundraiser_commerce_credit_card_cardonfile_fields($donation);

  // Check the card number to see if it matches the current card data number
  // If it does then we use the update function
  // if its a new card check if we should create a new CIM profile
  $action = 'update';
  // If the card_number in $cardonfile_fields is FALSE, the number wasn't
  // entered, so the address might need updating.
  if ($cardonfile_fields['card_number'] && $cardonfile_fields['card_number'] != $card_data->card_number) {
    // This is a new card for this donation, make sure we don't have an existing profile
    // First check with the card number and date
    if ($existing = _fundraiser_commerce_cardonfile_match($cardonfile_fields)) {
      // Set the existing card on the donation and return it
      $donation->data['cardonfile'] = $existing->card_id;
      $card_data = $existing;
      return $card_data;
    }
    // Check if we have a mismatched exp date record
    elseif ($mismatch =  _fundraiser_commerce_cardonfile_exp_date_mismatch($cardonfile_fields)) {
      $donation->data['cardonfile'] = $mismatch->card_id;
      $card_data = $mismatch;
    }
    // Nothing stored locally so create a new profile
    else {
      $action = 'new';
    }
  }
  if ($action == 'update') {
    $donation->old_card = clone $card_data;

    if (!empty($cardonfile_fields['card_number'])) {
      $card_data->card_number = $cardonfile_fields['card_number'];
    }
    $card_data->card_exp_month = $cardonfile_fields['card_exp_month'];
    $card_data->card_exp_year = $cardonfile_fields['card_exp_year'];

    // We're mirroring the commerce_authnet_cim_cardonfile_update function so setup values that function uses, see that function for details
    $form['credit_card']['number']['#default_value'] = '';
    $form_state['values']['credit_card']['number'] = $pane_values['credit_card']['number'];
    $form_state['values']['credit_card']['owner'] = isset($donation->donation['first_name']) ? $donation->donation['first_name'] : '';
    $form_state['values']['credit_card']['owner'] .= isset($donation->donation['last_name']) ? ' ' . $donation->donation['last_name'] : '';
    $form_state['values']['credit_card']['address'] = isset($donation->donation['address']) ? $donation->donation['address'] : '';
    $form_state['values']['credit_card']['address'] .= isset($donation->donation['address_line_2']) ? ' ' . $donation->donation['address_line_2'] : '';
    $form_state['values']['credit_card']['city'] = isset($donation->donation['city']) ? $donation->donation['city'] : '';
    $form_state['values']['credit_card']['state'] = isset($donation->donation['state']) ? $donation->donation['state'] : '';
    $form_state['values']['credit_card']['zip'] = isset($donation->donation['zip']) ? $donation->donation['zip'] : '';
    $form_state['values']['credit_card']['country'] = isset($donation->donation['country']) ? $donation->donation['country'] : '';

    $response = _commerce_authnet_fundraiser_commerce_authnet_cim_cardonfile_update($form, $form_state, $payment_method, $card_data);

    if ($response) {
      commerce_cardonfile_save($card_data);
      return $card_data;
    }
  }
  // Create a new card on file record
  elseif ($action == 'new') {
    // We're using the built in function that requires a payment details array and the order
    $payment_details = array(
      'cardNumber' => $pane_values['credit_card']['number'],
      'expirationDate' => $pane_values['credit_card']['exp_year'] . '-' . sprintf("%02d", $pane_values['credit_card']['exp_month']),
    );

    if (isset($pane_values['credit_card']['code'])) {
      $payment_details['cardCode'] = $pane_values['credit_card']['code'];
    }

    // Load the order
    $order = commerce_order_load($donation->did);
    $response = commerce_authnet_cim_create_customer_profile_request($payment_method, $order, $payment_details);

    // If the Customer Profile creation was a success, store the new card on file data locally.
    if ($response && (string) $response->messages->resultCode == 'Ok') {
      // Build a remote ID that includes the Customer Profile ID and the
      // Payment Profile ID.
      $remote_id = (string) $response->customerProfileId . '|' . (string) $response->customerPaymentProfileIdList->numericString;

      // Get the card type
      $card_type = commerce_payment_validate_credit_card_type($pane_values['credit_card']['number'], array_keys(commerce_payment_credit_card_types()));
      $all_types = commerce_payment_credit_card_types();
      $card_type = !empty($all_types[$card_type]) ? $all_types[$card_type] : 'card';

      // Create a new cardonfile entity
      $new_data = commerce_cardonfile_new();
      $new_data->uid = $order->uid;
      $new_data->payment_method = $payment_method['method_id'];
      $new_data->instance_id = $payment_method['instance_id'];
      $new_data->remote_id = $remote_id;
      $new_data->card_type = $card_type;
      $new_data->card_name = !empty($card_data->card_name) ? $card_data->card_name : '';
      $new_data->card_number = $cardonfile_fields['card_number'];
      $new_data->card_exp_month = $cardonfile_fields['card_exp_month'];
      $new_data->card_exp_year = $cardonfile_fields['card_exp_year'];
      $new_data->status = 1;

      // Save and log the creation of the new card on file.
      commerce_cardonfile_save($new_data);
      // Set the value on the donation
      $donation->data['cardonfile'] = $new_data->card_id;
      watchdog('commerce_authnet', 'CIM Customer Profile @profile_id created and saved to user @uid.', array('@profile_id' => (string) $response->customerProfileId, '@uid' => $order->uid));
      return $new_data;
    }
  }
  return $card_data;
}

/**
 * COPY: commerce_authnet_cim_cardonfile_update
 * Card on file callback: updates the associated customer payment profile.
 *
 * The original function at commerce_authnet_cim_cardonfile_update(), doesn't update the user
 * address fields - instead referencing the cim values saved, so not even updating the customer profile
 * in advance would do it. So we need to copy this over and make the changes we need for fall through.
 */
function _commerce_authnet_fundraiser_commerce_authnet_cim_cardonfile_update($form, &$form_state, $payment_method, $card_data) {
  // Extract the Customer Profile and Payment Profile IDs from the remote_id.
  list($cim_customer_profile_id, $cim_payment_profile_id) = explode('|', $card_data->remote_id);

  if (!empty($form['credit_card']['number']['#default_value']) && $form_state['values']['credit_card']['number'] != $form['credit_card']['number']['#default_value']) {
    $number = $form_state['values']['credit_card']['number'];
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
  if (!empty($form_state['values']['credit_card']['owner'])) {
    $name_parts = explode(' ', $form_state['values']['credit_card']['owner']);
    $first_name = array_shift($name_parts);
    $last_name = implode(' ', $name_parts);
    $address = $form_state['values']['credit_card']['address'];
    $city = $form_state['values']['credit_card']['city'];
    $state = $form_state['values']['credit_card']['state'];
    $zip = $form_state['values']['credit_card']['zip'];
    $country = $form_state['values']['credit_card']['country'];
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
        'creditCard' => array(
          'cardNumber' => $number,
          'expirationDate' => $card_data->card_exp_year . '-' . sprintf("%02d", $card_data->card_exp_month),
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
 * Refund callback for authnet, wraps the admin form used for refund with our own system.
 *
 * Here are some commonly seen response codes sent by authnet:
 * 1 = This transaction has been approved.
 * 11 = Duplicate transaction; for example, submitting two, partial refunds of $5.01 in a row.
 * 54 = "The referenced transaction does not meet the criteria for issuing a credit".
 *      The most likely explanation for response code 54 is that the original payment
 *      in question has not been "settled" by authorize.net, which can take 24 hours.
 *      response code "54" can be caused by other issues as well:
 *        - The original payment has been settled more than 120 days.
 *        - Invalid type (A valid type would be "credit")
 *        - invalid reference transaction ID
 *        - Wrong currency
 *
 * For more: http://developer.authorize.net/api/reference/dist/json/responseCodes.json
 */
function commerce_authnet_fundraiser_commerce_refund($payment_method, $refund) {
  $refund_was_successful = FALSE;

  // Retrieve data.
  $refunding_order = commerce_order_load($refund->did);
  $transactions = commerce_payment_transaction_load_multiple(array(), $conditions = array('order_id' => $refunding_order->order_id));
  $transaction = array_shift($transactions); // The first one is the original donation.

  // Set up the values needed for the transaction to go through.
  $form = array();
  $form_state = array(
   'transaction' => $transaction,
   'values' => array('amount' => $refund->amount),
   'order' => $refunding_order,
   'payment_method' => commerce_payment_method_instance_load($transaction->instance_id),
  );

  // Submit the donation form.
  module_load_include('inc', 'commerce_authnet', '/includes/commerce_authnet.admin');
  $result = commerce_authnet_aim_credit_form_submit($form, $form_state);
  // At this point, the card has been saved on the user profile.

  // Next, check the newly saved authnet payload's response code to verify the refund a success:
  $transactions = commerce_payment_transaction_load_multiple(array(), $conditions = array('order_id' => $refunding_order->order_id));
  $transaction = array_pop($transactions);
  $payload = array_pop($transaction->payload);

  // Get authnet's response code from the payload:
  $response_code_index = 2;
  $response_code = (string) $payload[$response_code_index];

  // Add additional information for common authnet response codes:
  switch ($response_code) {
    case '11':
      drupal_set_message('Authorize.net rejected this refund because an identical refund was recently issued against this transaction.' . '<br />' .
        'If this duplicate refund was intentional then please wait 3-5 minutes before trying it again.', 'error');
      break;
    case '54':
      drupal_set_message('New transactions are settled once per day based on your Authorize.net account\'s "Transaction Cut-Off Time".' . '<br />' .
        'If you change the cut-off time after today\'s transactions have been settled, the new cut-off time will be used tomorrow.', 'error');
      break;
  }

  // Correct commerce message typo if present:
  $error_messages = drupal_get_messages('error');
  if (isset($error_messages['error']) && is_array($error_messages['error'])) {
    foreach ($error_messages['error'] as $message) {
      if ($message == 'The transaction must be setted before a credit can be issued. This usually takes 24 hours') {
        $message = 'The transaction must be settled before a credit can be issued. This usually takes 24 hours';
      }
      drupal_set_message($message, 'error');
    }
  }

  // A response code of 1 indicates success AND $result will
  // be FALSE if commerce_authnet detects any problems:
  if ($response_code === '1' && $result !== FALSE) {
    $refund_was_successful = TRUE;
  }

  return $refund_was_successful;
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
function commerce_authnet_form_rules_ui_edit_element_builder(&$form, &$form_state) {
  // The following two "if" checks are probably not needed, but retained for
  // readability purposes.
  if (isset($form['parameter']['payment_method']['settings']['payment_method']['method_id']['#value'])) {
    $id = $form['parameter']['payment_method']['settings']['payment_method']['method_id']['#value'];
    $is_com = isset($form['parameter']['commerce_order']) ? 1 : 0;
    if ($id == 'authnet_aim' && $is_com) {
      // Check if we're in the commerce_ui or rules UI admin pages.
      if (arg(4) == 'reaction' && arg(5) == 'manage') {
        // We're in the rules UI.
        $rule = rules_config_load(arg(6));
      }
      elseif (arg(3) == 'payment-methods' && arg(4) == 'manage') {
        // We're in the commerce payment method UI.
        $rule = rules_config_load(arg(5));
      }

      // @TODO: is there a better way to get the rule?
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
          if (isset($action->settings['payment_method']['method_id']) && $action->settings['payment_method']['method_id'] == 'authnet_aim' && $action->elementId() == $action_id) {
            // @TODO: is there a better way to get the action ID?
            $settings[] = $action->settings;
          }
        }
        $config = isset($settings[0]['payment_method']['settings']) ? $settings[0]['payment_method']['settings'] : '';

        // Add a fieldset of Authorize.net request fields
        // available to have Webform field data mapped to them.
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['value_map'] = array(
          '#type' => 'fieldset',
          '#title' => t('Submission mapping'),
          '#description' => t('Enter the webform field keys to map to these authorize.net fields.'),
          '#tree' => TRUE,
        );
        $available_fields = _commerce_authnet_extra_fields();
        unset($available_fields[0]);
        $i = 1;
        foreach ($available_fields as $field) {
          $form['parameter']['payment_method']['settings']['payment_method']['settings']['commerce_authnet_webform_field_' . $i] = array(
            '#title' => t('Enter webform field #%i', array('%i' => $i)),
            '#type' => 'textfield',
            '#description' => t('Example: field_name'),
            '#default_value' => !empty($config['commerce_authnet_webform_field_' . $i]) ? $config['commerce_authnet_webform_field_' . $i] : '',
          );

          $form['parameter']['payment_method']['settings']['payment_method']['settings']['commerce_authnet_authorize_field_' . $i] = array(
            '#title' => t('Authorize field #%i', array('%i' => $i)),
            '#type' => 'select',
            '#options' => _commerce_authnet_extra_fields(),
            '#default_value' => !empty($config['commerce_authnet_authorize_field_' . $i]) ? $config['commerce_authnet_authorize_field_' . $i] : '',
          );
          $i++;
        }

        // Add a fieldset of Authorize.net response
        // fields available to be mapped to donation fields.
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['response_fields'] = array(
          '#type' => 'fieldset',
          '#title' => t('Response Fields for Mapping'),
          '#description' => t('The selected fields will be made available for mapping to exported donations.'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
          '#element_validate' => array('fundraiser_commerce_response_fields_validate'),
        );
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['response_fields']['mappable'] = array(
          '#type' => 'value',
          '#value' => array(),
        );
        $form['parameter']['payment_method']['settings']['payment_method']['settings']['response_fields']['all'] = array(
          '#theme' => 'table',
          '#header' => array(
            'checkbox' => t('Available'),
            'description' => t('Authorize.net gateway response field'),
          ),
          '#rows' => array(),
        );
        foreach (_commerce_authnet_response_fields() as $machine_name => $field) {
          $id = 'mappable_fields-' . $machine_name;
          $form['parameter']['payment_method']['settings']['payment_method']['settings']['response_fields']['all'][$machine_name] = array(
            '#id' => $id,
            '#type' => 'checkbox',
            '#default_value' => in_array($machine_name, !empty($config['response_fields']['mappable']) ? $config['response_fields']['mappable'] : array()),
          );
          $form['parameter']['payment_method']['settings']['payment_method']['settings']['response_fields']['all']['#rows'][$machine_name] = array(
            'checkbox' => array(
              // Tricky but it works: The table element's children
              // (the checkboxes) will all have rendered as
              // form components by the time the table theme function picks
              // them up by reference here.
              'data' => &$form['parameter']['payment_method']['settings']['payment_method']['settings']['response_fields']['all'][$machine_name],
            ),
            'description' => '<label for="' . $id . '">' . $field['name'] . '</label><span>' . t($field['description']) . '</span>',
          );
        }
      }
    }
  }
}

function fundraiser_commerce_commerce_authnet_aim_request_alter(&$nvp) {
  $donation = fundraiser_donation_get_donation($nvp['x_invoice_num']);
  $config = $donation->gateway['gateway_details']['settings'];
  $fields = _commerce_authnet_extra_fields();
  unset($fields[0]);
  $i = 1;
  foreach ($fields as $field) {
    if (isset($config['commerce_authnet_webform_field_' . $i]) && isset($donation->submission_data[$config['commerce_authnet_webform_field_' . $i]])) {
      if (isset($nvp[$fields[$config['commerce_authnet_authorize_field_' . $i]]])) {
        unset($nvp[$fields[$config['commerce_authnet_authorize_field_' . $i]]]);
      }
      $nvp += array(
        $fields[$config['commerce_authnet_authorize_field_' . $i]] => $donation->submission_data[$config['commerce_authnet_webform_field_' . $i]],
      );
    }
    $i++;
  }
}

function _commerce_authnet_extra_fields() {
  $extra_fields = array(
   '', // Nothing.
    // Normal overrides.
    'x_company',            // Up to 50 characters (no symbols)
    'x_ship_to_company',    // Up to 50 characters (no symbols)
    // Shipping fields, not recommended if your site needs to ship anything.
    'x_ship_to_first_name', // Up to 50 characters (no symbols)
    'x_ship_to_last_name',  // Up to 50 characters (no symbols)
    'x_ship_to_address',    // Up to 60 characters (no symbols)
    'x_ship_to_city',       // Up to 40 characters (no symbols)
    'x_ship_to_state',      // Up to 40 characters (no symbols) or a valid two-character state code
    'x_ship_to_zip',        // Up to 20 characters (no symbols)
    'x_ship_to_country',    // Up to 60 characters (no symbols)
  );
  return $extra_fields;
}

/**
 * Callback for fundraiser_commerce to discover what gateway response fields are available for mapping.
 *
 * @param array $gateway_info
 */
function commerce_authnet_fundraiser_commerce_response_fields($gateway_info) {
  $mappable_fields = array();
  $all_fields = _commerce_authnet_response_fields();
  if (!empty($gateway_info['gateway_details']['settings']['response_fields']['mappable'])) {
    foreach ($gateway_info['gateway_details']['settings']['response_fields']['mappable'] as $machine_name) {
      $mappable_fields[$machine_name] = $all_fields[$machine_name];
    }
  }
  return $mappable_fields;
}

/**
 * Callback to save gateway response fields when a donation is successfully completed.
 *
 * @param stdClass $donation
 */
function commerce_authnet_fundraiser_commerce_response_save($donation) {
  // We only need to save the response fields once.
  if (!db_query(
    "SELECT COUNT(*) FROM {fundraiser_commerce_gateway_response} WHERE did=:did AND gateway_id=:gateway_id",
    array(':did' => $donation->did, ':gateway_id' => $donation->gateway['id'])
  )->fetchField()) {
    // Find the raw gateway response string in the Commerce transaction.
    $transaction = commerce_payment_transaction_load($donation->result['data']['transaction_id']);
    $payload = $transaction->payload[ $transaction->created ];
    if (!empty($payload)) {
      foreach (_commerce_authnet_response_fields() as $machine_name => $field_info) {
        $field_data = (object)array(
          'did' => $donation->did,
          'gateway_id' => $donation->gateway['id'],
          'field_name' => $machine_name,
          'value' => $payload[ $field_info['index'] ],
        );
        drupal_write_record('fundraiser_commerce_gateway_response', $field_data);
      }
    }
  }
}

/**
 * Definition of all Authorize.net response fields available for mapping.
 *
 * Derived from http://www.authorize.net/support/merchant/wwhelp/wwhimpl/common/html/wwhelp.htm#context=Merchant_Integration_Guide&file=4_TransResponse.html
 * Note that the actual payloads we get from AIM contain more fields than documented by Authorize.net,
 * and the credit card fields' numeric indeces don't match the documentation.
 */
function _commerce_authnet_response_fields() {
  return array(
    'response_code' => array(
      'index' => 0,
      'name' => t('Response Code'),
      'description' => t('The overall status of the transaction.'),
    ),
    'response_subcode' => array(
      'index' => 1,
      'name' => t('Response Subcode'),
      'description' => t('A code used by the payment gateway for internal transaction tracking. Note: This field applies to CNP AIM only, and is obsolete.'),
    ),
    'response_reason_code' => array(
      'index' => 2,
      'name' => t('Response Reason Code'),
      'description' => t('A code that represents more details about the result of the transaction.'),
    ),
    'response_reason_text' => array(
      'index' => 3,
      'name' => t('Response Reason Text'),
      'description' => t('A brief description of the result, which corresponds with the response reason code.'),
    ),
    'authorization_code' => array(
      'index' => 4,
      'name' => t('Authorization Code'),
      'description' => t('The authorization or approval code.'),
    ),
    'avs_reponse' => array(
      'index' => 5,
      'name' => t('AVS Response'),
      'description' => t('The Address Verification Service (AVS) response code.'),
    ),
    'transaction_id' => array(
      'index' => 6,
      'name' => t('Transaction ID'),
      'description' => t('The payment gateway assigned identification number for the transaction.'),
    ),
    'invoice_number' => array(
      'index' => 7,
      'name' => t('Invoice Number'),
      'description' => t('The merchant assigned invoice number for the transaction.'),
    ),
    'description' => array(
      'index' => 8,
      'name' => t('Description'),
      'description' => t('The transaction description.'),
    ),
    'amount' => array(
      'index' => 9,
      'name' => t('Amount'),
      'description' => t('The amount of the transaction.'),
    ),
    'method' => array(
      'index' => 10,
      'name' => t('Method'),
      'description' => t('The payment method.'),
    ),
    'transaction_type' => array(
      'index' => 11,
      'name' => t('Transaction Type'),
      'description' => t('The type of credit card transaction.'),
    ),
    'customer_id' => array(
      'index' => 12,
      'name' => t('Customer ID'),
      'description' => t('The merchant assigned customer ID.'),
    ),
    'first_name' => array(
      'index' => 13,
      'name' => t('First Name'),
      'description' => t("The first name associated with the customer’s billing address."),
    ),
    'last_name' => array(
      'index' => 14,
      'name' => t('Last Name'),
      'description' => t("The last name associated with the customer’s billing address."),
    ),
    'company' => array(
      'index' => 15,
      'name' => t('Company'),
      'description' => t("The company associated with the customer’s billing address."),
    ),
    'address' => array(
      'index' => 16,
      'name' => t('Address'),
      'description' => t("The customer’s billing address."),
    ),
    'city' => array(
      'index' => 17,
      'name' => t('City'),
      'description' => t("The city of the customer’s billing address."),
    ),
    'state' => array(
      'index' => 18,
      'name' => t('State'),
      'description' => t("The state of the customer’s billing address."),
    ),
    'zip' => array(
      'index' => 19,
      'name' => t('ZIP Code'),
      'description' => t("The ZIP code of the customer’s billing address."),
    ),
    'country' => array(
      'index' => 20,
      'name' => t('Country'),
      'description' => t("The country of the customer’s billing address."),
    ),
    'phone' => array(
      'index' => 21,
      'name' => t('Phone'),
      'description' => t("The phone number associated with the customer’s billing address."),
    ),
    'fax' => array(
      'index' => 22,
      'name' => t('Fax'),
      'description' => t("The fax number associated with the customer’s billing address."),
    ),
    'email' => array(
      'index' => 23,
      'name' => t('Email Address'),
      'description' => t("The customer’s valid email address."),
    ),
    'ship_first_name' => array(
      'index' => 24,
      'name' => t('Ship To First Name'),
      'description' => t("The first name associated with the customer’s shipping address."),
    ),
    'ship_last_name' => array(
      'index' => 25,
      'name' => t('Ship To Last Name'),
      'description' => t("The last name associated with the customer’s shipping address."),
    ),
    'ship_company' => array(
      'index' => 26,
      'name' => t('Ship To Company'),
      'description' => t("The company associated with the customer’s shipping address."),
    ),
    'ship_address' => array(
      'index' => 27,
      'name' => t('Ship To Address'),
      'description' => t("The customer’s shipping address."),
    ),
    'ship_city' => array(
      'index' => 28,
      'name' => t('Ship To City'),
      'description' => t("The city of the customer’s shipping address."),
    ),
    'ship_state' => array(
      'index' => 29,
      'name' => t('Ship To State'),
      'description' => t("The state of the customer’s shipping address."),
    ),
    'ship_zip' => array(
      'index' => 30,
      'name' => t('Ship To ZIP Code'),
      'description' => t("The ZIP code of the customer’s shipping address."),
    ),
    'ship_country' => array(
      'index' => 31,
      'name' => t('Ship To Country'),
      'description' => t("The country of the customer’s shipping address."),
    ),
    'tax' => array(
      'index' => 32,
      'name' => t('Tax'),
      'description' => t('The tax amount charged.'),
    ),
    'duty' => array(
      'index' => 33,
      'name' => t('Duty'),
      'description' => t('The duty amount charged.'),
    ),
    'freight' => array(
      'index' => 34,
      'name' => t('Freight'),
      'description' => t('The freight amount charged.'),
    ),
    'tax_exempt' => array(
      'index' => 35,
      'name' => t('Tax Exempt'),
      'description' => t('The tax exempt status.'),
    ),
    'purchase_order_number' => array(
      'index' => 36,
      'name' => t('Purchase Order Number'),
      'description' => t('The merchant assigned purchase order number.'),
    ),
    'md5_hash' => array(
      'index' => 37,
      'name' => t('MD5 Hash'),
      'description' => t('The payment gateway generated MD5 hash value that can be used to authenticate the transaction response.'),
    ),
    'account_number' => array(
      'index' => 50,
      'name' => t('Account Number'),
      'description' => t('Last 4 digits of the card provided.'),
    ),
    'card_type' => array(
      'index' => 51,
      'name' => t('Card Type'),
      'description' => t('Visa, MasterCard, American Express, Discover, Diners Club, EnRoute, JCB.'),
    ),
  );
}
