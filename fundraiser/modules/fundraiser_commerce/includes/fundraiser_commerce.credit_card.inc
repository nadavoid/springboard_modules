<?php

/**
 * @file
 * Credit-card helper functions for fundraiser commerce.
 */

/**
 * Returns a set of credit card form elements
 *
 * @return Array
 *    Form elements ready to be included on donation form
 */
function _fundraiser_commerce_credit_card_form() {
  // Move expiration date values up in the array. ONLY relevant for the default credit card fields.
  $form['card_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Credit card number'),
    '#attributes' => array('class' => array('input-large')),
    '#size' => 20,
    '#fundraiser_commerce_required' => TRUE,
    '#salesforce_label' => t('Credit card last 4'),
  );
  $form['expiration_date']['#theme'] = 'fundraiser_credit_card_expiration_date';
  $form['expiration_date']['#title'] = t('Expiration date');
  $months[1] = 'January';
  $months[2] = 'February';
  $months[3] = 'March';
  $months[4] = 'April';
  $months[5] = 'May';
  $months[6] = 'June';
  $months[7] = 'July';
  $months[8] = 'August';
  $months[9] = 'September';
  $months[10] = 'October';
  $months[11] = 'November';
  $months[12] = 'December';
  $form['expiration_date']['card_expiration_month'] = array(
    '#type' => 'select',
    '#options' => $months,
    '#title' => t('Expiration month'),
    '#attributes' => array('class' => array('input-small')),
    '#default_value' => date('n'),
    '#fundraiser_commerce_required' => TRUE,
  );
  $this_year = date('Y');
  $years[$this_year] = $this_year;
  for ($i = 1; $i <= 15; $i++) {
    $years[$this_year + $i] = $this_year + $i;
  }
  $form['expiration_date']['card_expiration_year'] = array(
    '#type' => 'select',
    '#options' => $years,
    '#title' => t('Expiration year'),
    '#attributes' => array('class' => array('input-small')),
    '#default_value' => date('Y'),
    '#fundraiser_commerce_required' => TRUE,
  );
  $form['card_cvv'] = array(
    '#type' => 'textfield',
    '#title' => t('CVV'),
    '#size' => 6,
    '#attributes' => array('class' => array('input-small')),
    '#fundraiser_commerce_required' => TRUE,
    '#salesforce_mappable' => FALSE,
  );

  // Hidden field to be filled in during processing.
  $form['card_type'] = array(
    '#type' => 'hidden',
    '#title' => 'Card type',
    '#value' => FALSE,
  );

  return $form;
}

/**
 * Remove or change values of sensitive data from the submitted data
 *
 * @param $submission_fields Array
 *    Values from the submitted form
 *
 * @return Array
 *    The updated submission_fields array
 */
function _fundraiser_commerce_credit_card_cleanup($submission_fields) {
  if (isset($submission_fields['payment_fields']['credit']['expiration_date'])) {
    if (isset($submission_fields['payment_fields']['credit']['expiration_date']['card_expiration_month'])) {
      $submission_fields['payment_fields']['credit']['card_expiration_month'] =
        $submission_fields['payment_fields']['credit']['expiration_date']['card_expiration_month'];
    }
    if (isset($submission_fields['payment_fields']['credit']['expiration_date']['card_expiration_year'])) {
      $submission_fields['payment_fields']['credit']['card_expiration_year'] =
        $submission_fields['payment_fields']['credit']['expiration_date']['card_expiration_year'];
    }
    unset($submission_fields['payment_fields']['credit']['expiration_date']);
  }

  // Add the credit card type if possible
  if (!empty($submission_fields['payment_fields']['credit']['card_number'])) {
    module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
    $card_type = commerce_payment_validate_credit_card_type($submission_fields['payment_fields']['credit']['card_number'], array_keys(commerce_payment_credit_card_types()));
    $all_types = commerce_payment_credit_card_types();
    $submission_fields['payment_fields']['credit']['card_type'] = !empty($all_types[$card_type]) ? $all_types[$card_type] : NULL;
  }
  return $submission_fields;
}

/**
 * Return the credit card expiration dates
 *
 * @param array $submission_fields
 *   Values from the submitted form.
 * @param array $form_parents
 *   The parent field names.
 * @param boolean $require_cvv
 *   Set to FALSE to skip checking the cvv code, defaults to TRUE.
 */
function _fundraiser_commerce_credit_card_validate($submission_fields, $form_parents, $require_cvv = TRUE) {
  $prefix = implode('][', $form_parents) . '][';

  // If using a stored card, return early.
  if (isset($submission_fields['payment_fields']['token']) && $submission_fields['payment_fields']['token']) {
    return;
  }

  // Include the file with the validation info.
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
  $number = $submission_fields['payment_fields']['credit']['card_number'];
  if (empty($number)) {
    form_set_error($prefix . 'card_number', t('The credit card number is required.'));
  }
  else {
    $valid = commerce_payment_validate_credit_card_number($number);
    if ($valid !== TRUE || !ctype_digit($number) || drupal_strlen($number) < 10) {
      form_set_error($prefix . 'card_number', t('You have entered an invalid credit card number.'));
    }
  }

  if ($require_cvv) {
    $cvv = $submission_fields['payment_fields']['credit']['card_cvv'];
    if (empty($cvv)) {
      form_set_error($prefix . 'card_cvv', t('The credit card CVV is required.'));
    }
    else {
      $valid = commerce_payment_validate_credit_card_security_code($number, $cvv);
      if ($valid !== TRUE && !empty($cvv)) {
        form_set_error($prefix . 'card_cvv', t('You have entered an invalid CVV number.'));
      }
    }
  }

  $cc_expiration_month = $submission_fields['payment_fields']['credit']['card_expiration_month'];
  $cc_expiration_year = $submission_fields['payment_fields']['credit']['card_expiration_year'];
  if (empty($cc_expiration_month) || empty($cc_expiration_year)) {
    form_set_error($prefix . 'expiration_date][card_expiration_month', t('The credit card expiration date is required.'));
    form_set_error($prefix . 'expiration_date][card_expiration_year');
  }
  else {
    $valid = commerce_payment_validate_credit_card_exp_date($cc_expiration_month, $cc_expiration_year);
    if ($valid !== TRUE && !empty($cc_expiration_month) && !empty($cc_expiration_year)) {
      form_set_error($prefix . 'expiration_date][card_expiration_month', t('The credit card you entered has expired.'));
      form_set_error($prefix . 'expiration_date][card_expiration_year');
    }
  }
}

/**
 * Return the credit card expiration dates
 *
 * @param $submission_fields Array
 *    Values from the submitted form
 *
 * @return Array
 *    Expiration dates
 */
function _fundraiser_commerce_credit_card_expire($submission_fields) {
  $expires = array();
  if (isset($submission_fields['payment_fields']['credit']['card_expiration_month'])) {
    $expires['month'] = $submission_fields['payment_fields']['credit']['card_expiration_month'];
  }
  if (isset($submission_fields['payment_fields']['credit']['card_expiration_year'])) {
    $expires['year'] = $submission_fields['payment_fields']['credit']['card_expiration_year'];
  }
  return $expires;
}

/**
 * Return the credit card expiration dates
 *
 * @param $submission_fields Array
 *    Values from the submitted form
 *
 * @return Array
 *    Expiration dates
 */
function _fundraiser_commerce_credit_card_expires_today($submission_fields) {
  $expires = array(
    'month' => date('n'),
    'year' => date('Y'),
  );
  return $expires;
}

/**
 * Scrub the data
 *
 * @param $payment_method String
 *    Credit, check, paypal, etc
 * @param $fields Array
 *    Values for the credit card
 *
 * @return Array
 *    The updated fields values
 */
function _fundraiser_commerce_credit_card_scrub($fields) {
  // Scrub sensitive donation fields if they exists.
  if (isset($fields['credit']['card_number'])) {
    // Reduce the credit card number to last four digits
    $fields['credit']['card_number'] = substr(
      $fields['credit']['card_number'], -4);
  }
  if (isset($fields['credit']['card_cvv'])) {
    $fields['credit']['card_cvv'] = '';
  }
  return $fields;
}

/**
 * Returns the pane values for commerce from the fundraiser credit card form
 *
 * @param $donation Object
 *    Fundraiser donation object
 *
 * @return Array
 *    Values ready to be passed to the commerce payment processor
 */
function _fundraiser_commerce_credit_card_pane_values($donation) {
  return array(
    'credit_card' => array(
      'number' => isset($donation->donation['payment_fields']['credit']['card_number']) ?
        $donation->donation['payment_fields']['credit']['card_number'] : '',
      'exp_month' => isset($donation->donation['payment_fields']['credit']['card_expiration_month']) ?
        $donation->donation['payment_fields']['credit']['card_expiration_month'] : '',
      'exp_year' => isset($donation->donation['payment_fields']['credit']['card_expiration_year']) ?
        $donation->donation['payment_fields']['credit']['card_expiration_year'] : '',
      'code' => isset($donation->donation['payment_fields']['credit']['card_cvv']) ?
        $donation->donation['payment_fields']['credit']['card_cvv'] : '',
    ),
  );
}

/**
 * Returns the values need to load a card on file profile
 *
 * @param $donation Object
 *    Fundraiser donation object
 *
 * @return Array
 *    Values ready to be passed into a card on file profile query
 */
function _fundraiser_commerce_credit_card_cardonfile_fields($donation) {
  // Use the pane values function to the get the cc details
  $pane_values = _fundraiser_commerce_credit_card_pane_values($donation);

  return array(
    // Strip the card number down to the last four since thats all card on file will store
    'card_number' => substr($pane_values['credit_card']['number'], -4),
    'card_exp_month' => $pane_values['credit_card']['exp_month'],
    'card_exp_year' => $pane_values['credit_card']['exp_year'],
  ) + array('instance_id' => $donation->gateway['id'], 'uid' => $donation->uid);
}
