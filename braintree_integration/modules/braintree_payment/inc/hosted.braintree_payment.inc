<?php
/**
 * @file
 * Holds code for hosted fields on Braintree payment form.
 */

/**
 * Implements hook_braintree_payment_form__PAYMENT_TYPE__alter().
 */
function braintree_payment_braintree_payment_form__credit__alter(&$context) {
  $payment_method_instance = $context['payment_method_instance'];
  if ($payment_method_instance['method_id'] != 'braintree_hostedfields') {
    return;
  }

  $form = &$context['form'];
  $form_state = &$context['form_state'];
  $js_settings = &$context['js_settings'];

  $form['braintree']['new']['number'] = array(
    '#type' => 'item',
    '#title' => t('Credit card number'),
    '#markup' => '<div id="card-number" class="braintree-hosted-field required"></div>',
    '#id' => 'braintree-field-card-number',
    '#prefix' => '<div class="control-group">',
    '#suffix' => '</div>',
  );

  $form['braintree']['new']['expiration'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('braintree-card-expiration'),
    ),
  );

  $form['braintree']['new']['expiration']['month'] = array(
    '#type' => 'item',
    '#title' => t('Expiration Month'),
    '#markup' => '<div id="expiration-month" class="braintree-hosted-field control-group required"></div>',
  );

  $form['braintree']['new']['expiration']['divider'] = array(
    '#markup' => '<span class="braintree-month-year-divider">/</span>',
  );

  $form['braintree']['new']['expiration']['year'] = array(
    '#type' => 'item',
    '#title' => t('Year'),
    '#markup' => '<div id="expiration-year" class="braintree-hosted-field control-group required"></div>',
  );

  $form['braintree']['new']['cvv'] = array(
    '#title' => '<a href="#" class="tooltip"><div class="tooltip__text">' . t('The 3 or 4 digit security code on your card') . '</div></a>',
    '#type' => 'item',
    '#title' => t('CVV'),
    '#markup' => '<div id="cvv" class="braintree-hosted-field control-group required"></div>',
    '#id' => 'braintree-field-card-cvv',
  );

  $js_settings['enabledMethods'][] = 'credit';
  $js_settings['integration'] = 'custom';
  $js_settings['fieldsStyles'] = braintree_hostedfields_styles();
  $js_settings['hostedFields'] = array(
    'number' => array(
      'selector' => '#card-number',
      'placeholder' => '1111 1111 1111 1111',
    ),
    'cvv' => array(
      'selector' => '#cvv',
      'placeholder' => '123',
    ),
    'expirationMonth' => array(
      'selector' => '#expiration-month',
      'placeholder' => 'MM',
      'select' => TRUE,
    ),
    'expirationYear' => array(
      'selector' => '#expiration-year',
      'placeholder' => 'YYYY',
      'select' => TRUE,
    ),
  );
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function braintree_payment_form_alter(&$form, &$form_state, $form_id) {
  $braintree = !empty($form['submitted']['payment_information']['payment_fields']['credit']['braintree']);
  $offline = !empty($form['#node']->fundraiser_offline);
  if ($braintree && $offline) {
    $form['submitted']['payment_information']['payment_fields']['credit']['braintree']['new']['number']['#required'] = TRUE;
    $form['submitted']['payment_information']['payment_fields']['credit']['braintree']['new']['expiration']['month']['#required'] = TRUE;
    $form['submitted']['payment_information']['payment_fields']['credit']['braintree']['new']['expiration']['year']['#required'] = TRUE;
    $form['submitted']['payment_information']['payment_fields']['credit']['braintree']['new']['cvv']['#required'] = FALSE;
  }
}
