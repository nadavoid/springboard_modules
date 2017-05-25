<?php


function springboard_group__cardonfile__menu_alter(&$items) {
  if (springboard_group_commerce_orders_are_grouped()) {
    foreach ($items as $path => $item) {
      if ($path == 'user/%views_arg/cards') {
        $items[$path]['access callback'] = 'springboard_group_commerce_cardonfile_views_access';
      }
      if (isset($items[$path]['access callback']) && $items[$path]['access callback'] == 'commerce_cardonfile_access') {
        $items[$path]['access callback'] = 'springboard_group_commerce_cardonfile_access';
      }
    }
  }
}

/**
 * Add the order-related permissions.
 *
 * Sub-hook to springboard_group_og_permissions() hook.
 */
function springboard_group__commerce_cardonfile__og_permissions(&$items) {
  $orders_grouped = springboard_group_commerce_orders_are_grouped();
  if (!empty($orders_grouped)) {

    $items['edit any card data in springboard group'] = array(
      'title' => t('Edit any card data in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_cardonfile',
    );

    $items['view any card data in springboard group'] = array(
      'title' => t('View any card data in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_cardonfile',
    );

    $items['create card data in springboard group'] = array(
      'title' => t('Create card data  in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_cardonfile',
    );

    $items['delete any card data in springboard group'] = array(
      'title' => t('Delete any card data in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_cardonfile',
    );
  }
}

function springboard_group_commerce_cardonfile_access($op = 'view', $card = NULL, $account = NULL) {

  global $user;
  $account = isset($account) ? $account : $user;
  if ($op == 'manage' || $op == 'update') {
    $op = 'edit';
  }

  $payment_method = array();
  $payment_rule = NULL;
  if (!empty($card->instance_id)) {
    $payment_method = commerce_payment_method_instance_load($card->instance_id);
    if (!empty($payment_method)) {
      list($payment_method_id, $instance_rule_name) = explode('|', $payment_method['instance_id']);
      $payment_rule = rules_config_load($instance_rule_name);
    }
  }
  if (empty($payment_method) && $op != 'view') {
    return FALSE;
  }

  switch ($op) {
    case 'edit':
      $op_callback = commerce_cardonfile_payment_method_callback($payment_method, 'update callback');
      break;

    case 'create':
      $op_callback = commerce_cardonfile_payment_method_callback($payment_method, 'create callback');
      if ($op_callback && !empty($payment_rule) && empty($payment_rule->active)) {
        return FALSE;
      }
      break;

    case 'delete':
      $op_callback = commerce_cardonfile_payment_method_callback($payment_method, 'delete callback');
      break;

    default:
      $op_callback = TRUE;
      break;
  }

  if (empty($op_callback)) {
    return FALSE;
  }

  if ((user_access('administer card data') || user_access("$op any card data")) && user_access('administer group')) {
    return TRUE;
  }

  // og checks here.

  // If we get here, use the standard access check.
  return commerce_cardonfile_access($op, $card, $account);
}

/**
 * Determines if the current user has access to the account's stored cards.
 */
function springboard_group_commerce_cardonfile_user_access($account) {
  global $user;
  if ((user_access('administer card data') || user_access('view any card data')) && user_access('administer group')) {
    return TRUE;
  }
  else {
     // og checks here
  }

  // If we get here, use the standard access check.
  return commerce_cardonfile_user_access($account);
}

function springboard_group_commerce_cardonfile_views_access($args) {
  if (!user_access('administer group') && $args[0] == 'views_check_perm') {
    return springboard_group_user_has_perm_in_any_group('view any card data in springboard group');
  }
  return views_access($args);
}