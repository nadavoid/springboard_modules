<?php

/**
 * @file
 * Springboard Quick Donation admin functions.
 */

/**
 * Admin settings form.
 */
function fundraiser_quick_donate_admin_settings($form, $form_state) {
  $form['default_forms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default Forms'),
    '#description' => t('When a user is directed to a donation form from an HMAC token with an unsupported payment gateway, they will be redirected to a default form that does support the payment gateway.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $gateways = _fundraiser_gateway_info();
  // List of payment gateways that shouldn't be configured ever.
  $blacklist = array(
    'commerce_sage_payments_eft|commerce_payment_commerce_sage_payments_eft',
    'authnet_echeck|commerce_payment_authnet_echeck',
    'commerce_litle_echeck|commerce_payment_commerce_litle_echeck',
    'commerce_psi_eft|commerce_payment_commerce_psi_eft',
    'commerce_sage_payments_eft|commerce_payment_commerce_sage_payments_eft',
    // @todo Remove once braintree is supported.
    'braintree_hostedfields|commerce_payment_braintree_hostedfields',
    'braintree_paypal|commerce_payment_braintree_paypal',
    'braintree_applepay|commerce_payment_braintree_applepay',
    'authnet_echeck|rules_authorize_net_echeck_net_avv_2016_authorize_net_sandbox_',
  );
  $gateways = array_filter($gateways, function($element) use ($blacklist) {
    return !(in_array($element['id'], $blacklist) || empty($element['gateway_details']['cardonfile']));
  });

  $gateway_keys = array_keys($gateways);
  foreach ($gateway_keys as $gateway_key) {
    $default = '';
    $data = variable_get(FUNDRAISER_QUICK_DONATE_DEFAULT_FORM_PREFIX . $gateway_key, array());
    if (!empty($data)) {
      $data = unserialize($data);
      $default = $data['title'];
    }
    $form['default_forms'][FUNDRAISER_QUICK_DONATE_DEFAULT_FORM_PREFIX . $gateway_key] = array(
      '#type' => 'textfield',
      '#title' => t('Default node ID for @gateway', array(
        '@gateway' => isset($gateways[$gateway_key]['title']) ? $gateways[$gateway_key]['title'] : $gateway_key,
      )),
      '#description' => t('The node ID of the form you want to be used as default.'),
      '#autocomplete_path' => 'admin/springboard/options/fundraiser/fundraiser_quick_donate/autocomplete/' . $gateway_key,
      '#default_value' => $default,
    );
  }

  $data = variable_get(FUNDRAISER_QUICK_DONATE_DEFAULT_FORM_PREFIX . 'default', array());
  if (!empty($data)) {
    $data = unserialize($data);
    $default = $data['title'];
  }
  $form['default_forms'][FUNDRAISER_QUICK_DONATE_DEFAULT_FORM_PREFIX . 'default'] = array(
    '#type' => 'textfield',
    '#title' => t('Default node ID for all gateways'),
    '#description' => t('The node ID of the form you want to be used as default.'),
    '#autocomplete_path' => 'admin/springboard/options/fundraiser/fundraiser_quick_donate/autocomplete/all',
    '#default_value' => $default,
  );

  $form['default_messages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default Messages'),
    '#description' => t('Configure the default quick donate messages that will be displayed to the end user.'),
  );
  $form['default_messages']['fundraiser_quick_donate_default_optin_message_new_user'] = array(
    '#type' => 'textfield',
    '#title' => t('"Quick Donation" opt-in text (for users not already set up with Quick Donate)'),
    '#description' => t('This is the title of the opt-in checkbox. The "@type" placeholder can be used to specify the type of payment method (either credit card or bank account). Defaults to "Save this @type for future use".'),
    '#default_value' => variable_get('fundraiser_quick_donate_default_optin_message_new_user', t('Save this @type for future use')),
  );
  $form['default_messages']['fundraiser_quick_donate_default_help_message_new_user'] = array(
    '#type' => 'text_format',
    '#title' => t('"Quick Donation" help text (for users not already set up with Quick Donate)'),
    '#description' => t('This message will be shown to users to provide information about quick donations. Use !type to print the type of payment method, which will be either credit or bank account.<br/><br/>', array('!type' => '<strong>@type</strong>')),
    '#default_value' => variable_get('fundraiser_quick_donate_default_help_message_new_user', t('Save your @type details and donate faster next time. We may email you from time to time with quick donate opportunities. You can update your @type details or cancel your enrollment at any time.')),
    '#format' => variable_get('fundraiser_quick_donate_default_help_message_new_user_format', 'plain_text'),
  );
  $form['default_messages']['fundraiser_quick_donate_default_optin_message_existing_user'] = array(
    '#type' => 'textfield',
    '#title' => t('"Quick Donation" opt-in text (for users already set up with Quick Donate)'),
    '#description' => t('This is the title of the opt-in checkbox. The "@type" placeholder can be used to specify the type of payment method (either credit card or bank account). Defaults to "Save this @type for future use".'),
    '#default_value' => variable_get('fundraiser_quick_donate_default_optin_message_existing_user', t('Update your saved @type?')),
  );
  $form['default_messages']['fundraiser_quick_donate_default_help_message_existing_user'] = array(
    '#type' => 'text_format',
    '#title' => t('"Quick Donation" help text (for users already set up with Quick Donate)'),
    '#description' => t('This message will be shown to users to provide information about quick donations. Use !type to print the type of payment method, which will be either credit or bank account.<br/><br/>', array('!type' => '<strong>@type</strong>')),
    '#default_value' => variable_get('fundraiser_quick_donate_default_help_message_existing_user', t('Check this box if you want to make this new @type your default for future donations.')),
    '#format' => variable_get('fundraiser_quick_donate_default_help_message_existing_user_format', 'plain_text'),
  );
  $form['default_messages']['fundraiser_quick_donate_default_login_message'] = array(
    '#type' => 'textfield',
    '#title' => t('"Quick Donation" login text'),
    '#description' => t('This is the title of the log-in checkbox. The "@type" placeholder can be used to specify the type of payment method (credit card or bank account). Defaults to "Already have a saved @type?".'),
    '#default_value' => variable_get('fundraiser_quick_donate_default_login_message', t('Already have a saved @type?')),
  );
  $form['default_messages']['fundraiser_quick_donate_default_login_link_message'] = array(
    '#type' => 'textfield',
    '#title' => t('"Quick Donation" login link text'),
    '#description' => t('This is the title of the log-in checkbox. Defaults to "Click here to login".'),
    '#default_value' => variable_get('fundraiser_quick_donate_default_login_link_message', t('Click here to login')),
  );
  /*$form['default_messages']['fundraiser_quick_donate_default_default_message'] = array(
    '#type' => 'textfield',
    '#title' => t('"Set as default card" text'),
    '#description' => t('The text of the "Set as default card" checkbox, which asks users if they want this card they are adding to be their default card for future payments. Defaults to "Set as your default payment method?"'),
    '#default_value' => variable_get('fundraiser_quick_donate_default_default_message', t('Set as your default payment method?')),
  );*/

  // @todo Uncomment the following when we're ready to allow multiple cards to
  // be used for quick donating.
  // $form['other'] = array(
  //   '#type' => 'fieldset',
  //   '#title' => t('Other options'),
  // );
  // $form['other']['fundraiser_quick_donate_allow_multiple_cards'] = array(
  //   '#type' => 'checkbox',
  //   '#title' => t('Allow multiple stored cards to be configured for quick donations'),
  //   '#default_value' => variable_get('fundraiser_quick_donate_allow_multiple_cards', FALSE),
  // );

  $form['#submit'][] = 'fundraiser_quick_donate_admin_settings_submit';

  return system_settings_form($form);
}

/**
 * Autocomplete callback function for fundraiser_quick_donate_admin_settings().
 */
function fundraiser_quick_donate_admin_settings_autocomplete_callback($gateway_key = 'all', $string = '') {
  // Using db_select here because we need to also join the node table.
  $result = db_select('node', 'n')
    ->fields('n', array('nid', 'title'));
  if ($gateway_key != 'all') {
    $result->join('fundraiser', 'f', 'f.nid = n.nid');
    $result = $result->fields('f', array('gateways'))
      ->condition('f.gateways', '%' . db_like($gateway_key) . '%', 'LIKE');
  }
  $types = array_keys(fundraiser_get_donation_types());
  $result = $result->condition('n.type', $types, 'IN');
  if (is_numeric($string)) {
    $result = $result->condition('n.nid', db_like($string) . '%', 'LIKE');
  }
  else {
    $result = $result->condition('n.title', db_like($string) . '%', 'LIKE');
  }
  $result = $result->range(0, 10)
    ->execute()
    ->fetchAll();
  $matches = array();
  foreach ($result as $node) {
    $matches[$node->title . " [$node->nid]"] = check_plain($node->title) . " [nid: $node->nid]";
  }
  drupal_json_output($matches);
}

/**
 * Submission handler for fundraiser_quick_donate_admin_settings().
 */
function fundraiser_quick_donate_admin_settings_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, FUNDRAISER_QUICK_DONATE_DEFAULT_FORM_PREFIX) === FALSE || empty($value)) {
      continue;
    }
    $match = preg_match('/.*\[([0-9]+)\].*/', $value, $matches);
    if (!$match || $match !== FALSE && count($matches) < 2) {
      if (!is_numeric($value)) {
        continue;
      }

      // Try to get the title based on just the node ID.
      $query = db_query('SELECT title FROM {node} WHERE nid = :nid', array(
        ':nid' => $value,
      ))->fetchField();

      if (empty($query)) {
        continue;
      }

      $form_state['values'][$key] = $query . ' [' . $value . ']';
      $matches[1] = $value;
    }
    $data = array(
      'title' => $form_state['values'][$key],
      'nid' => $matches[1],
    );
    $form_state['values'][$key] = serialize($data);
  }
  $form_state['values']['fundraiser_quick_donate_default_help_message_new_user_format'] = $form_state['values']['fundraiser_quick_donate_default_help_message_new_user']['format'];
  $form_state['values']['fundraiser_quick_donate_default_help_message_new_user'] = $form_state['values']['fundraiser_quick_donate_default_help_message_new_user']['value'];
  $form_state['values']['fundraiser_quick_donate_default_help_message_existing_user_format'] = $form_state['values']['fundraiser_quick_donate_default_help_message_existing_user']['format'];
  $form_state['values']['fundraiser_quick_donate_default_help_message_existing_user'] = $form_state['values']['fundraiser_quick_donate_default_help_message_existing_user']['value'];
}

/**
 * Upgrade link generator form.
 */
function fundraiser_quick_donate_token_generator($form, $form_state) {
  $key = springboard_hmac_get_key();
  if (empty($key)) {
    drupal_set_message(t('Secure Prepopulate key and initialization vector are not set. Upgrade links will not work.'), 'error');
  }

  $defaults = array();
  $default_defaults = array(
    'uid' => 1,
    'card_id' => 1,
    'gateway_id' => 'authnet_aim|commerce_payment_authnet_aim',
  );
  foreach ($default_defaults as $key => $default) {
    if (isset($_SESSION['qd_token_generate'][$key])) {
      $defaults[$key] = $_SESSION['qd_token_generate'][$key];
    }
    else {
      $defaults[$key] = $default;
    }
  }

  $form['fundraiser_quick_donate_springboard_hmac'] = array(
    '#type' => 'fieldset',
    '#title' => t('Quick Donate Token'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('To test quick donation tokens, enter the following field information and press "Generate Token".'),
  );

  $form['fundraiser_quick_donate_springboard_hmac']['uid'] = array(
    '#type' => 'textfield',
    '#title' => t('User ID'),
    '#size' => 20,
    '#default_value' => $defaults['uid'],
    '#description' => t('The UID of the donator.'),
  );

  $form['fundraiser_quick_donate_springboard_hmac']['card_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Card ID'),
    '#description' => t('The card ID.'),
    '#default_value' => $defaults['card_id'],
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate Token'),
  );

  $form['#submit'][] = 'fundraiser_quick_donate_token_generator_validate';
  $form['#submit'][] = 'fundraiser_quick_donate_token_generator_submit';

  $form['#attached']['js'][] = drupal_get_path('module', 'fundraiser_quick_donate') . '/js/fundraiser_quick_donate.admin.js';

  return $form;
}

/**
 * Helper function to get card id's for a user.
 *
 * @param int $uid
 *   The uid of the user.
 */
function _fundraiser_quick_donate_admin_get_card_ids_by_uid($uid) {
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'commerce_cardonfile')
    ->propertyCondition('uid', $uid)
    ->propertyCondition('status', 1);
  $result = $query->execute();
  drupal_json_output(array_keys($result['commerce_cardonfile']));
}

/**
 * Store temporary upgrade link values in session.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function fundraiser_quick_donate_token_generator_validate($form, $form_state) {
  $_SESSION['qd_token_generate'] = $form_state['values'];
}

/**
 * Create an upgrade link.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function fundraiser_quick_donate_token_generator_submit($form, $form_state) {
  $payload = array(
    'uid' => $form_state['values']['uid'],
    'card_id' => $form_state['values']['card_id'],
  );

  $link = fundraiser_quick_donate_create_link($payload);

  drupal_set_message('Quick donate link generated! ' . l($link, $link));
}
