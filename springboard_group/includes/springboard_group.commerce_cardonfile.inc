<?php


function springboard_group__cardonfile__menu_alter(&$items) {
  if (springboard_group_users_are_grouped()) {
    foreach ($items as $path => $item) {
      if ($path == 'user/%views_arg/cards') {
        $items[$path]['access callback'] = 'springboard_group_commerce_cardonfile_views_access';
      }
      if (isset($item['access callback']) && $item['access callback'] === 'commerce_cardonfile_access') {
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
  $users_grouped = springboard_group_users_are_grouped();
  if (!empty($users_grouped)) {

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

  // Check if the gateway can do this op.
  $op_callback = springboard_group_cardonfile_check_gateway_callbacks($op, $card, $payment_method, $payment_rule);
  // @todo gateways callbacks can implement access checks!
  if (empty($op_callback)) {
    return FALSE;
  }

  $gids = springboard_group_get_contact_group_gids($account->uid);

  if (!empty($gids) && ((user_access('administer card data') || user_access("$op any card data")) && user_access('administer group'))) {
    return TRUE;
  }
  elseif (!empty($gids)) {
    // If the user in the group that owns the gateway,
    // and also has edit cardonfile
    // permission in that group, allow the edit???
    foreach ($gids as $gid) {
      $access = springboard_group_check_group_perm($gid, $op . ' any card data in springboard group');
      if ($access) {
        return TRUE;
      }
    }
    return FALSE;
  }

  return commerce_cardonfile_access($op, $card, $account);
}

/**
 * Determines if the current user has access to the account's stored cards.
 *
 * Used by views plugin. Can disallow access to stored cards tab.
 */
function springboard_group_commerce_cardonfile_user_access($account) {
  global $user;
  if ((user_access('administer card data') || user_access('view any card data')) && user_access('administer group')) {
    return TRUE;
  }
  else {
    // Are users grouped?
    if (springboard_group_users_are_grouped()) {
      $gids = springboard_group_get_contact_group_gids($account->uid);
      foreach ($gids as $gid) {
        if (springboard_group_check_group_perm($gid, 'administer users in springboard group')) {
          $user_access[] = $gid;
        }
      }
      if (!empty($user_access)) {
        foreach ($user_access as $gid) {
          if (springboard_group_check_group_perm($gid, 'view any card data in springboard group')) {
            return TRUE;
          }
        }
      }
      // Check card's gateway's group?
    }
    else {
      // Contacts aren't grouped, see if admin has card view
      // rights in any group.
      $access = springboard_group_user_has_perm_in_any_group('view any card data in springboard group');
      if ($access) {
        return TRUE;
      }
    }
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


function springboard_group_cardonfile_check_gateway_callbacks($op, $card, $payment_method, $payment_rule) {

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
  return $op_callback;
}



function springboard_group_query_views_commerce_card_on_file_user_cards_alter(&$view, &$query) {
  // Skip this if a user is a group admin.
  if (!user_access('administer group')) {
    // Check if orders are grouped.
    if (springboard_group_rules_config_are_grouped()) {
       // @todo alter the landing page to show only eligible cards.
    }
  }
}

// If the contact does not belong to any of the admins groups or the admin
// does not have administer user permission i the group, I assume the admin
// would not have access to /user/* paths, including cardonfile. If the
// contact is in the admin’s group, and the admin can administer users and
// administer cardonfile, I assume that also only cards related to gateways
// that the admin’s group owns will display. It seems complicated.