<?php
/**
 * @file
 * Fundraiser functions for springboard groups.
 */

/**
 * Update the gateway form after AJAX reload triggered by group selection.
 *
 * Called from springboard_group_node_form_alter().
 */
function springboard_group_fundraiser_node_form__alter(&$form, &$form_state, $callback) {
  // An ajax reload has occurred.
  if (!empty($form_state['values'])) {
    if ($callback != 'springboard_group_og_group_field_ajax_callback' && !empty($callback)) {
      return;
    }
  }
  // Add the ajax wrapper to the form prior to Ajax reload.
  $form['payment_methods']['gateways']['#prefix'] = '<div id = "springboard-groups-payment-ajax-wrapper">';
  $form['payment_methods']['gateways']['#suffix'] = '</div>';
  $form['#validate'][] = 'springboard_group_fundraiser_node_form_gateway_validate_gateway';
  springboard_group_fundraiser_rebuild_form($form, $form_state);

}

/**
 * Validation hook checks for gateway/group mismatch.
 */
function springboard_group_fundraiser_node_form_gateway_validate_gateway($form, $form_state) {
  $gateways = springboard_group_fundraiser_check_gateways($form, $form_state);
  $selected_gateways = $form_state['values']['gateways'];
  foreach ($selected_gateways as $gateway) {
    if (isset($gateway['status']) && !isset($gateways[$gateway['id']])) {
      form_set_error('gateways', t('Invalid gateway selected. Gateway selection does not match group selection.'));
      springboard_group_fundraiser_rebuild_form($form, $form_state);
    }
  }
}

/**
 * Helper function to load valid gateways.
 */
function springboard_group_fundraiser_check_gateways($form, $form_state) {
  // Get the group ID from the form values.
  $language = $form['og_group_ref_' . $form['#node']->type]['#language'];
  if (!empty($form_state['values'])) {
    $option = $form_state['values']['og_group_ref_' . substr($form['#node']->type, 0, 19)][$language];
    $gid = isset($option[0]['target_id']) ? $option[0]['target_id'] : '_none';
  }
  else {
    $option = $form['og_group_ref_' . substr($form['#node']->type, 0, 19)][$language][0]['default'];
    $gid = !empty($option['#default_value'][0]) ? $option['#default_value'][0] : '_none';
  }
  // Get the gateways and filter them by group ownership.
  $all_gateways = _fundraiser_gateway_info();
  $gateways = springboard_group_fundraiser_filter_gateways($all_gateways, $gid);

  return $gateways;
}

function springboard_group_fundraiser_rebuild_form(&$form, &$form_state) {
  $gateways = springboard_group_fundraiser_check_gateways($form, $form_state);
  $node = $form['#node'];
  $lost_gateways = [];
  if (!empty($node->gateways)) {
    foreach ($node->gateways as $key => $gateway) {
      if (isset($gateway['id']) && !in_array($gateway['id'], array_keys($gateways))) {
        $lost_gateways[] = $gateway;
      }
    }
  }
  // Rebuild the gateway fieldset. This code block is lifted directly from
  // _fundraiser_donation_settings_form() with a slight alteration.
  if (empty($gateways)) {
    $message = t('There are no gateways configured for this group. Please create some gateways in the store configuration.');
    $form_state['group_ajax_messages'][] = $message;

    // Slight alteration to suppress php notice about
    // missing form state values.
    $form['payment_methods']['gateways'] = array(
      '#type' => 'hidden',
      '#value' => array(),
    );
    $form['payment_methods']['gateways']['_default'] = array(
      '#type' => 'hidden',
      '#value' => array(),
    );
  }
  else {
    if (!empty($lost_gateways)) {
      $num = count($lost_gateways);
      $method = format_plural(count($lost_gateways), 'method', 'methods');
      $list = [];
      foreach ($lost_gateways as $lost_gateway) {
        list(, $name) = explode('|', $lost_gateway['id']);
        $rule = rules_config_load($name);
        if (!empty($rule)) {
          $list[] = l(check_plain($rule->label), 'admin/commerce/config/payment-methods/manage/' . $rule->id, array('attributes' => array('target' => '_blank')));
        }
      }
      $gateways_message = theme('item_list', array('items' => $list));
      $message = t('@num payment @method will be removed: !gateways', array(
        '@num' => $num,
        '@method' => $method,
        '!gateways' => $gateways_message,
      ));
      $form_state['group_ajax_messages'][] = $message;
    }
    $fieldset = array(
      '#type' => 'fieldset',
      '#title' => t('Payment methods'),
      '#description' => t('Enable donation form payment methods and their corresponding gateways.'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
      '#theme' => 'fundraiser_form_payment_methods',
    );
    $configuration = !empty($node->gateways) ? $node->gateways : variable_get('fundraiser_payment_method_config_defaults', array());
    $form['payment_methods']['gateways'] = _fundraiser_form_payment_method_config_fieldset($gateways, $configuration, $fieldset, 'gateways');
  }
  // Add the ajax wrapper back to the rebuilt fieldset.
  $form['payment_methods']['gateways']['#prefix'] = '<div id = "springboard-groups-payment-ajax-wrapper">';
  $form['payment_methods']['gateways']['#suffix'] = '</div>';
  if (empty($gateways)) {
    $form['payment_methods']['gateways']['#prefix'] .= '<div>' . $message . '</div>';
  }
}

/**
 * Check group affiliation of gateways.
 *
 * @param array $gateways
 *   Existing payment gateways defiened in hook_fundraiser_gateway_info().
 * @param int $gid
 *   The group ID.
 *
 * @return mixed
 *   The gateways filtered by group.
 */
function springboard_group_fundraiser_filter_gateways($gateways, $gid = NULL) {
  $group_methods = array();
  $no_group_methods = array();

  foreach ($gateways as $key => $gateway) {

    // Get the gateway's rule configuraiton and checked if it's grouped.
    $name = explode('|', $key);
    $rule = rules_config_load($name[1]);
    $rule_gid = NULL;
    if (!empty($rule->og_group_ref_rules_config[LANGUAGE_NONE][0])) {
      foreach ($rule->og_group_ref_rules_config[LANGUAGE_NONE] as $gateway) {
        $rule_gid = $gateway['target_id'];
        if ($gid == $rule_gid) {
          $group_methods[$key] = $key;
        }
      }
    }
    else {
      // No group on payment method.
      $no_group_methods[$key] = $key;
    }

    if ($gid == '_none' && !in_array($key, $no_group_methods)) {
      unset($gateways[$key]);
    }
    elseif ($gid != '_none' && !in_array($key, $group_methods)) {
      unset($gateways[$key]);
    }
  }

  return $gateways;
}

function springboard_group_fundraiser_donation_create($donation) {
  $grouped_types = springboard_group_get_grouped_types();
  foreach ($grouped_types as $entity_type => $bundle) {
    $function = 'springboard_group__' . $entity_type . '__fundraiser_donation_create';
    if (function_exists($function)) {
      $function($donation);
    }
  }
}

/**
 * Add the order-related permissions.
 *
 * Sub-hook to springboard_group_og_permissions() hook.
 */
function springboard_group__fundraiser__og_permissions(&$items) {
  $orders_grouped = springboard_group_commerce_orders_are_grouped();
  if (!empty($orders_grouped)) {

    $items['submit springboard group donations offline'] = array(
      'title' => t('Submit donations offline in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'fundraiser',
    );

    $items['cancel future donations in springboard group'] = array(
      'title' => t('Cancel future donations in in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'fundraiser',
    );
    $items['fastforward recurring donations in springboard group'] = array(
      'title' => t('Fast-forward recurring donations in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'fundraiser',
    );

//  $items['administrate recurring donations'] = array(
//    'title' => t('Administer recurring donations'),
//    'description' => '',
//    'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
//    'default role' => array(),
//  );

  }
}