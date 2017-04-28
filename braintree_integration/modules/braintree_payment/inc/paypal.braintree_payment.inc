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

/**
 * Implements hook_module_implements_alter().
 */
function braintree_payment_module_implements_alter(&$implementations, $hook) {
  // Move our hook to the end of the list to ensure fundraiser has had a chance
  // to add its overrides.
  if ($hook == 'form_node_form_alter' || $hook == 'form_alter') {
    $group = $implementations['braintree_payment'];
    unset($implementations['braintree_payment']);
    $implementations['braintree_payment'] = $group;
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Add Paypal 'compact' option to donation forms using the Braintree Paypal
 * payment method.
 */
function braintree_payment_form_node_form_alter(&$form, &$form_state, $form_id) {
  if (!_braintree_payment_paypal_method_is_option($form['#node'])) {
    return;
  }

  // If this is an appropriate node edit/add form, add the "paypal compact"
  // checkbox to the "Payment methods" fieldset.
  if (empty($form['payment_methods']['gateways']) || !element_get_visible_children($form['payment_methods']['gateways'])) {
    return;
  }

  // Is Paypal Braintree an available payment method?
  if (isset($form['payment_methods']['gateways']['paypal']) && array_key_exists('braintree_paypal|commerce_payment_braintree_paypal', $form['payment_methods']['gateways']['paypal']['id']['#options'])) {
    $form['payment_methods']['braintree_paypal_compact'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable PayPal compact flow on this donation form?'),
      '#description' => t('If enabled, the default fundraiser donor information and billing fields will be hidden and automatically populated by Paypal.'),
      '#default_value' => _braintree_payment_paypal_method_is_enabled($form['#node']),
      '#states' => array(
        'visible' => array(
          ':input[name="gateways[paypal][status]"]' => array('checked' => TRUE),
          'select[name="gateways[paypal][id]"]' => array('value' => 'braintree_paypal|commerce_payment_braintree_paypal'),
        ),
      ),
      '#parents' => array('gateways', 'paypal', 'compact'),
    );
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function braintree_payment_form_webform_component_edit_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'webform_component_edit_form') {
    if (isset($form['nid']) && isset($form['nid']['#value'])) {
      $node = node_load($form['nid']['#value']);
      // Need to do some additional component validation to prevent breaking
      // the compact flow.
      if (_braintree_payment_paypal_method_is_option($node) && _braintree_payment_paypal_method_is_enabled($node)) {
        $form['#validate'][] = '_braintree_payment_webform_components_validate';
      }
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function braintree_payment_form_webform_components_form_alter(&$form, &$form_state) {
  if (isset($form['#node']) && _braintree_payment_paypal_method_is_option($form['#node']) && _braintree_payment_paypal_method_is_enabled($form['#node'])) {
    // On the components overview page, need to workaround an issue with the
    // weights (which determine the components position) not being updated after
    // a validation error.
    if (!empty($form_state['input'])) {
      drupal_add_js(drupal_get_path('module', 'braintree_payment') . '/js/braintree_payment.paypal.admin.js');
    }
    $form['#validate'][] = '_braintree_payment_webform_components_validate';
  }
}

/**
 * Validation for component placement and type when compact flow is enabled.
 *
 * This does double duty for both the individual component add/edit page
 * and the components overview page.
 */
function _braintree_payment_webform_components_validate($form, &$form_state) {
  if ($form['#form_id'] == 'webform_component_edit_form') {
    $node = node_load($form['nid']['#value']);
    $billing_cid = webform_get_cid($node, 'billing_information', 0);
    $donor_cid = webform_get_cid($node, 'donor_information', 0);
    if (isset($form_state['values']['pid']) && ($form_state['values']['pid'] == $billing_cid || $form_state['values']['pid'] == $donor_cid) && isset($form_state['values']['type'])  && $form_state['values']['type'] != 'hidden' && $form_state['values']['form_key'] != 'sbp_phone') {
      form_set_error($form_state['values']['form_key'], t('The component <em>@key</em> cannot be added to this fieldset when Paypal compact flow is enabled', array('@key' => $form_state['values']['form_key'])));
    }
    return;
  }

  // Check the component position and type against the default fundraiser
  // fields. If there's a non-hidden field present in either of the default
  // fieldsets, flag it.
  $default_fundraiser_fields = _braintree_payment_default_fundraiser_fields();
  $billing_cid = webform_get_cid($form['#node'], 'billing_information', 0);
  $billing_children = _braintree_payment_webform_components_find_parent_children($billing_cid, $form_state['values']['components']);
  $donor_cid = webform_get_cid($form['#node'], 'donor_information', 0);
  $donor_children = _braintree_payment_webform_components_find_parent_children($donor_cid, $form_state['values']['components']);
  $children = array_merge($donor_children, $billing_children);
  foreach ($children as $cid) {
    $component = _braintree_payment_webform_get_component_by_cid($form['#node']->nid, $cid);
    if (!in_array($component->form_key, $default_fundraiser_fields) && $component->type != 'hidden') {
      form_set_error('components][' . $component->cid, t('The component <em>@key</em> cannot be added to this fieldset when Paypal compact flow is enabled', array('@key' => $component->form_key)));
      $errors[] = $component;
    }
  }

  if (isset($errors) && !empty($errors)) {
    drupal_add_js(array('paypalCompactErrors' => $errors), 'setting');
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Merge into braintree js settings on Braintree Paypal enabled donation forms.
 */
function braintree_payment_form_webform_client_form_alter(&$form, &$form_state) {
  if (empty($form['#node']->type) || !fundraiser_is_donation_type($form['#node']->type)) {
    return;
  }
  if (_braintree_payment_paypal_method_is_enabled($form['#node'])) {
    // TODO: refactor this duplicate code from validation. Ideally call the
    // validate function.
    $default_fundraiser_fields = _braintree_payment_default_fundraiser_fields();
    $billing_cid = webform_get_cid($form['#node'], 'billing_information', 0);
    $donor_cid = webform_get_cid($form['#node'], 'donor_information', 0);
    $billing_children = _braintree_payment_webform_components_find_parent_children($billing_cid, $form['#node']->webform['components']);
    $donor_children = _braintree_payment_webform_components_find_parent_children($donor_cid, $form['#node']->webform['components']);
    $children = array_merge($donor_children, $billing_children);

    // If compact flow is enabled display a warning if there are disallowed
    // components present.
    foreach ($children as $cid) {
      $component = _braintree_payment_webform_get_component_by_cid($form['#node']->nid, $cid);
      if (!in_array($component->form_key, $default_fundraiser_fields) && $component->type != 'hidden') {
        if (user_access('edit any donation form')) {
          drupal_set_message(t('The component <em>@key</em> in its current position will prevent the Paypal compact flow from displaying properly.', array('@key' => $component->form_key)), 'error');
        }
      }
    }

    $settings = array(
      'paypal' => array(
        'compact' => TRUE,
      ),
    );

    if (isset($form['#node']->form_layouts)) {
      $settings['paypal']['form_layout'] = $form['#node']->form_layouts;
    }

    drupal_add_js(array('braintree' => $settings), 'setting');
    drupal_add_js(drupal_get_path('module', 'braintree_payment') . '/js/braintree_payment.paypal.js');
  }
}

/**
 * Helper function to determine if the node has the braintree paypal option.
 */
function _braintree_payment_paypal_method_is_option($node, $form = NULL) {
  if (empty($node->type) || !fundraiser_is_donation_type($node->type)) {
    return FALSE;
  }
  if (isset($node->gateways) && isset($node->gateways['paypal']) && $node->gateways['paypal']['id'] == 'braintree_paypal|commerce_payment_braintree_paypal') {
    return TRUE;
  }
  return FALSE;
}

/**
 * Helper function.
 *
 * Check if a given node has the Paypal compact flow enabled.
 */
function _braintree_payment_paypal_method_is_enabled($node) {
  return !empty($node->gateways['paypal']['compact']);
}
