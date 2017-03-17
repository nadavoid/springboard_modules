<?php

/**
 * @file
 * Node form related hooks for Fundraiser One Click Donate.
 */

/**
 * Implements hook_form_alter().
 */
function fundraiser_ocd_form_node_form_alter(&$form, &$form_state, $form_id) {
  if (empty($form['#node']->type) || !fundraiser_is_donation_type($form['#node']->type)) {
    return;
  }

  // If this is an appropriate node edit/add form, add the "one-click donation"
  // checkbox to the "Payment methods" fieldset.
  if (empty($form['payment_methods']['gateways']) || !element_get_visible_children($form['payment_methods']['gateways'])) {
    return;
  }

  $form['payment_methods']['ocd'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable One Click Donate on this form?'),
    '#description' => t('Enabling One Click Donate will allow users to save their payment information for future use, login and use their saved card if they have one, and makes the form compatible with secure One Click Donate links.'),
    '#default_value' => isset($form['#node']->ocd) ? $form['#node']->ocd : FALSE,
  );

  if (empty(variable_get(FUNDRAISER_OCD_DEFAULT_FORM_PREFIX . 'default', array()))) {
    $form['payment_methods']['ocd']['#suffix'] = t("Reminder: Don't forget to set default forms on the !config_page so donors can be directed to the right place when following a secure One Click Donate link.<br/><br/>", array(
      '!config_page' => l(t('One Click Donate configuration page'), 'springboard/options/fundraiser/fundraiser_ocd'),
    ));
  }

  $usable_paypment_processors = array('credit' => array()/*, 'bank' => array()*/);
  if (!empty($form['payment_methods']['gateways']['credit'])) {
    foreach ($form['payment_methods']['gateways']['credit']['id']['#options'] as $gateway => $value) {
      // Check to see if the currently selected payment method is Card on
      // File enabled (via the cardonfile boolean in its info array).
      $payment_method = commerce_payment_method_instance_load($gateway);

      // Skip if payment method is not capable of card on file.
      if (!_commerce_cardonfile_capable_payment_method_check($payment_method) || !isset($payment_method['settings']['cardonfile']) || (isset($payment_method['settings']['cardonfile']) && !$payment_method['settings']['cardonfile'])) {
        continue;
      }

      // For onsite payment methods add a checkbox to the credit card
      // details container to store the credit card for future use.
      if (!$payment_method['offsite']) {
        $usable_paypment_processors['credit'][] = $gateway;
      }
    }
  }

  /*if (!empty($form['payment_methods']['gateways']['bank account'])) {
    foreach ($form['payment_methods']['gateways']['bank account']['id']['#options'] as $gateway => $value) {
      // Check to see if the currently selected payment method is Card on
      // File enabled (via the cardonfile boolean in its info array).
      $payment_method = commerce_payment_method_instance_load($gateway);

      // Skip if payment method is not capable of card on file.
      if (!_commerce_cardonfile_capable_payment_method_check($payment_method) || !isset($payment_method['settings']['cardonfile']) || (isset($payment_method['settings']['cardonfile']) && !$payment_method['settings']['cardonfile'])) {
        continue;
      }

      // For onsite payment methods add a checkbox to the credit card
      // details container to store the credit card for future use.
      if (!$payment_method['offsite']) {
        $usable_paypment_processors['bank'][] = $gateway;
      }
    }
  }*/

  if (empty($usable_paypment_processors['credit'])/* && empty($usable_paypment_processors['bank'])*/) {
    // If no usable payment processors are available, disable the checkbox
    // and display a note to the user.
    $form['payment_methods']['ocd']['#attributes']['disabled'] = TRUE;
    $form['payment_methods']['ocd']['#suffix'] = '<span class="note"><strong>Note:</strong> no usable payment processors were found, so one-click donations will not be available.</span>';
  }
  else {
    drupal_add_js(array(
      'fundraiser_ocd' => array(
        'usable_paypment_processors' => $usable_paypment_processors,
      ),
    ), 'setting');

    // Help text for new users.
    $form['payment_methods']['ocd_optin_message_new_user'] = array(
      '#type' => 'textfield',
      '#title' => t('"One Click Donation" opt-in text (for users not currently enrolled in One Click Donate)'),
      '#description' => t('This is the title of the opt-in checkbox. The "@type" placeholder can be used to specify the type of payment method (either credit card or bank account). Defaults to "Save this @type for future use".'),
      '#default_value' => isset($form['#node']->ocd_optin_message_new_user) ? $form['#node']->ocd_optin_message_new_user : variable_get('fundraiser_ocd_default_optin_message_new_user', t('Save this @type for future use')),
      '#states' => array(
        'visible' => array(
          ':input[name="ocd"]' => array('checked' => TRUE),
        ),
      ),
    );

    // Because a bug exists in D7 where the input filter help information
    // doesn't respect the '#states' property, we need to wrap the message
    // in a container and apply the #states property to the container
    // instead.
    // See https://www.drupal.org/node/997826 for more info.
    $form['payment_methods']['ocd_help_container_new_user'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="ocd"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['payment_methods']['ocd_help_container_new_user']['ocd_help_message_new_user'] = array(
      '#type' => 'text_format',
      '#title' => t('"One Click Donation" help text (for users not currently enrolled in One Click Donate)'),
      '#description' => t('This message will be shown to users to provide information about one-click donations. Use !type to print the type of payment method, which will be either credit or bank account.<br/><br/>', array('!type' => '<strong>@type</strong>')),
      '#default_value' => isset($form['#node']->ocd_help_message_new_user) ? $form['#node']->ocd_help_message_new_user : variable_get('fundraiser_ocd_default_help_message_new_user', t('Save your @type details and donate faster next time. We may email you from time to time with one-click donate opportunities. You can update your @type details or cancel your enrollment at any time.')),
      '#format' => isset($form['#node']->ocd_help_message_new_user_format) ? $form['#node']->ocd_help_message_new_user_format : variable_get('fundraiser_ocd_default_help_message_new_user_format', 'plain_text'),
    );

    // Help text for existing users.
    $form['payment_methods']['ocd_optin_message_existing_user'] = array(
      '#type' => 'textfield',
      '#title' => t('"One Click Donation" opt-in text (for users already enrolled in One Click Donate)'),
      '#description' => t('This is the title of the opt-in checkbox. The "@type" placeholder can be used to specify the type of payment method (either credit card or bank account). Defaults to "Save this @type for future use".'),
      '#default_value' => isset($form['#node']->ocd_optin_message_existing_user) ? $form['#node']->ocd_optin_message_existing_user : variable_get('fundraiser_ocd_default_optin_message_existing_user', t('Update your saved @type?')),
      '#states' => array(
        'visible' => array(
          ':input[name="ocd"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['payment_methods']['ocd_help_container_existing_user'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="ocd"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['payment_methods']['ocd_help_container_existing_user']['ocd_help_message_existing_user'] = array(
      '#type' => 'text_format',
      '#title' => t('"One Click Donation" help text (for users already enrolled in One Click Donate)'),
      '#description' => t('This message will be shown to users to provide information about one-click donations. Use !type to print the type of payment method, which will be either credit or bank account.<br/><br/>', array('!type' => '<strong>@type</strong>')),
      '#default_value' => isset($form['#node']->ocd_help_message_existing_user) ? $form['#node']->ocd_help_message_existing_user : variable_get('fundraiser_ocd_default_help_message_existing_user', t('Check this box if you want to make this new @type your default for future donations.')),
      '#format' => isset($form['#node']->ocd_help_message_existing_user_format) ? $form['#node']->ocd_help_message_existing_user_format : variable_get('fundraiser_ocd_default_help_message_existing_user_format', 'plain_text'),
    );

    $form['payment_methods']['ocd_login_message'] = array(
      '#type' => 'textfield',
      '#title' => t('"One Click Donation" login text'),
      '#description' => t('This is the title of the log-in checkbox. The "@type" placeholder can be used to specify the type of payment method (credit card or bank account). Defaults to "Already have a saved @type?".'),
      '#default_value' => isset($form['#node']->ocd_login_message) ? $form['#node']->ocd_login_message : variable_get('fundraiser_ocd_default_login_message', t('Already have a saved @type?')),
      '#states' => array(
        'visible' => array(
          ':input[name="ocd"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['payment_methods']['ocd_login_link_message'] = array(
      '#type' => 'textfield',
      '#title' => t('"One Click Donation" login link text'),
      '#description' => t('This is the title of the log-in checkbox. Defaults to "Click here to login".'),
      '#default_value' => isset($form['#node']->ocd_login_link_message) ? $form['#node']->ocd_login_link_message : variable_get('fundraiser_ocd_default_login_link_message', t('Click here to login')),
      '#states' => array(
        'visible' => array(
          ':input[name="ocd"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $form['actions']['submit']['#submit'][] = '_fundraiser_ocd_node_form_submit';
  $form['#attached']['js'][] = drupal_get_path('module', 'fundraiser_ocd') . '/js/fundraiser_ocd.node_form.js';
}

/**
 * Submission handler for node forms.
 *
 * Restructures array before it's passed to hook insert/update functions.
 */
function _fundraiser_ocd_node_form_submit($form, &$form_state) {
  if ($form_state['values']['ocd']) {
    // One Click Donate checkbox is checked, save OCD settings.
    $fields = array(
      'fundraiser_nid' => $form_state['nid'],
      'ocd_optin_message_new_user' => $form_state['values']['ocd_optin_message_new_user'],
      'ocd_help_message_new_user_format' => $form_state['values']['ocd_help_message_new_user']['format'],
      'ocd_help_message_new_user' => $form_state['values']['ocd_help_message_new_user']['value'],
      'ocd_optin_message_existing_user' => $form_state['values']['ocd_optin_message_existing_user'],
      'ocd_help_message_existing_user_format' => $form_state['values']['ocd_help_message_existing_user']['format'],
      'ocd_help_message_existing_user' => $form_state['values']['ocd_help_message_existing_user']['value'],
      'ocd_login_message' => $form_state['values']['ocd_login_message'],
      'ocd_login_link_message' => $form_state['values']['ocd_login_link_message'],
    );

    db_merge('fundraiser_ocd')
      ->key(array('fundraiser_nid' => $fields['fundraiser_nid']))
      ->fields($fields)
      ->execute();
  }
  else {
    // Not checked, delete any entries.
    db_delete('fundraiser_ocd')->condition('fundraiser_nid', $form_state['nid'], '=')->execute();
  }
}
