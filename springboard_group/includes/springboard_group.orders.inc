<?php
/**
 * @file
 * Orders definitions and hooks for Springboard Group.
 */

/**
 * Menu alter helper.
 *
 * The order payment paths are controlled by access functions which are not
 * alterable in the way we need, so we have to wrap the original access
 * functions in a custom access function. The custom access functions check
 * group-based access. If they find nothing, they forward the permission
 * check to the original access function.
 *
 * @param $items
 *
 * @see springboard_group_menu_alter()
 */
function springboard_group__commerce_order__menu_alter(&$items) {
  if (springboard_group_commerce_orders_are_grouped()) {
    $items['admin/commerce/orders/%commerce_order']['access callback'] = 'springboard_group_commerce_order_admin_order_view_access';
    $items['admin/commerce/orders/%commerce_order/payment']['access callback'] = 'springboard_group_commerce_payment_transaction_order_access';
    $items['admin/commerce/orders/%commerce_order/payment/%commerce_payment_transaction']['access callback'] = 'springboard_group_commerce_payment_transaction_access';
    $items['admin/commerce/orders/%commerce_order/payment/%commerce_payment_transaction/edit']['access callback'] = 'springboard_group_commerce_payment_transaction_access';
    $items['admin/commerce/orders/%commerce_order/payment/%commerce_payment_transaction/delete']['access callback'] = 'springboard_group_commerce_payment_transaction_access';
    $items['admin/reports/salesforce/donations']['access callback'] = 'springboard_group_donation_report_access';
  }
}

/**
 * Add the order-related permissions.
 *
 * Sub-hook to springboard_group_og_permissions() hook.
 */
function springboard_group__commerce_order__og_permissions(&$items) {
  $orders_grouped = springboard_group_commerce_orders_are_grouped();
  if (!empty($orders_grouped)) {

    $items['create orders in springboard group'] = array(
      'title' => t('Create  orders in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_order',
    );
    $items['edit any order in springboard group'] = array(
      'title' => t('Edit any order in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_order',
    );

    $items['administer orders in springboard group'] = array(
      'title' => t('Administer orders in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_order',
    );

    $items['view any order in springboard group'] = array(
      'title' => t('View any order in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_order',
    );

    $items['view payments in springboard group'] = array(
      'title' => t('View payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_payment',
    );

    $items['create payments in springboard group'] = array(
      'title' => t('Create payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_payment',
    );

    $items['delete payments in springboard group'] = array(
      'title' => t('Delete payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_payment',
    );

    $items['update payments in springboard group'] = array(
      'title' => t('Update payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'commerce_payment',
    );

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

/**
 * Quickly determine whether orders are grouped.
 */
function springboard_group_commerce_orders_are_grouped($fields = FALSE) {
  $group_enabled_field = &drupal_static(__FUNCTION__ . $fields);
  if (!isset($group_enabled_field)) {
    $group_enabled_field = og_get_group_audience_fields('commerce_order', 'commerce_order', 'node', 'springboard_group');
  }
  if (!$fields) {
    return (bool) $group_enabled_field;
  }
  else {
    return $group_enabled_field;
  }
}

/**
 * Add the group widget to the commerce order form.
 */
function springboard_group_form_commerce_order_ui_order_form_alter(&$form, &$form_state) {

  // No groups field, do not proceed.
  if (!springboard_group_commerce_orders_are_grouped()) {
    return;
  }

  // Convert the multiple select widget to a single selection widget.
  // See springboard_group_field_widget_form_alter() and
  // springboard_group_set_single_selector() for details about
  // why this is necessary.
  $fields = springboard_group_commerce_orders_are_grouped(TRUE);

  if (!empty($fields)) {
    // @todo make showing/hiding of group field configurable?
    if (1) {
      $groups = og_get_entity_groups('commerce_order', $form['#entity']);
      $form['og_group_ref_commerce_order']['#access'] = FALSE;
      $titles = [];
      if (!empty($groups['node'])) {
        foreach ($groups['node'] as $group_id) {
          $group = node_load($group_id);
          $titles[] = check_plain($group->title);
        }
        $form['order_group_display_holder'] = array(
          '#type' => 'fieldset',
          '#title' => t('Group Ownership'),
        );
        $form['order_group_display_holder']['order_group_display']['#markup'] = theme('item_list', array(
          'items' => $titles,
          'title' => '',
        ));
      }
    }
    else {

      springboard_group_set_single_selector($form, $fields);

      // Add ajax callback to the groups field so we can validate newly
      // selected groups.
      $language = $form['og_group_ref_commerce_order']['#language'];
      $form['og_group_ref_commerce_order'][$language][0]['default']['#ajax'] = array(
        'callback' => 'springboard_group_og_group_field_ajax_callback',
      );

      // Set up the validation modal window parameters.
      springboard_group_prepare_validation_modal($form, $form_state);

      // Add the validation handlers to the form.
      springboard_group_load_ajax_validation_handlers($form, $form_state);
    }
  }
}


/**
 * If the groups form element was exposed on orders, we'd need this.
 *
 * Currently non-functional.
 */
function springboard_group_commerce_order_form__validate(&$form, &$form_state, $callback = NULL) {
  if ($callback != 'springboard_group_og_group_field_ajax_callback') {
    return;
  }
  // @todo, if any.
}

/**
 * Implements hook_fundraiser_donation_create().
 *
 * Add orders to a group behind the scenes.
 */
function springboard_group__commerce_order__fundraiser_donation_create($donation) {

  // Check if commerce orders are grouped.
  if (springboard_group_commerce_orders_are_grouped()) {
    // If commerce orders are grouped, set the group id on new orders,
    // based on group ownership of the donation form.
    $node_wrapper = entity_metadata_wrapper('node', $donation->node);
    $group_field_name = 'og_group_ref_' . substr($donation->node->type, 0, 19);
    if (!empty($node_wrapper->{$group_field_name})) {
      $node_group = $node_wrapper->{$group_field_name}->value();
      $node_group_id = $node_group[0]->nid;
      // Get the original order.
      if (!empty($node_group_id)) {
        $order = commerce_order_load($donation->did);
        $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
        $order_wrapper->og_group_ref_commerce_order->set(array($node_group_id));
        $order_wrapper->save();
      }
    }
  }
}

/**
 * Implements hook_commerce_entity_access().
 *
 * Limit access to orders and payments based on group ownership of the order.
 */
function springboard_group_commerce_entity_access($op, $entity, $account, $entity_type) {
  // Check if orders are configured as groupable.
  if (springboard_group_commerce_orders_are_grouped()) {

    // Check for group update and delete permissions on the order entity.
    // View permissions are handled elsewhere. Entity can be an order or
    // a payment, etc.
    if (isset($entity)) {

      if (($op == 'update' || $op == 'delete')) {

        if ($entity_type == 'commerce_order') {
          $group = og_get_entity_groups('commerce_order', $entity);
        }
        // If the order has a group, check if the current user has access
        // in that group.
        if (!empty($group)) {
          // Check for global permissions.
          if (user_access('administer group')
          && user_access('administer orders')
          ) {
            return TRUE;
          }
          $gid = reset($group['node']);
          return springboard_group_check_group_perm($gid, 'edit any order in springboard group', $GLOBALS['user']->uid);
        }
        // The order isn't grouped. Only group admins can manage
        // non-grouped items.
        else {
          if (!user_access('administer group')
            && !user_access('administer orders')
          ) {
            return FALSE;
          }
        }
      }
    }
  }
}
/**
 * Implements hook_query_TAG_alter().
 *
 * Limit view access to commerce orders by group ownership.
 */
function springboard_group_query_commerce_order_access_alter(&$query) {
  // Skip this if a user is a group admin.
  if (!user_access('administer group')) {
    // Check if orders are grouped.
    if (springboard_group_commerce_orders_are_grouped()) {

      // Check if the query contains an Order ID.
      foreach ($query->conditions() as $condition) {
        if (!empty($condition['field']) && is_string($condition['field']) && $condition['field'] == 'order_id') {
          $id = $condition['value'];
        }
      }

      // If this is a listing, there is no order Id to check. We have to load
      // all orders in a group and check against the array.
      // @todo: really?
      if (empty($id)) {
        //$allowed_order_ids = springboard_group_orders_get_allowed_order_ids('view any order in springboard group');
        //$query->condition('fundraiser_donation.did', $allowed_order_ids, 'IN');
        //$query->condition('commerce_order.order_id', $allowed_order_ids, 'IN');
      }
      else {

        // Get the users groups.
        $order = commerce_order_load($id);
        $wrapper = entity_metadata_wrapper('commerce_order', $order);
        if ($wrapper->og_group_ref_commerce_order->raw()) {
          foreach ($wrapper->og_group_ref_commerce_order->raw() as $gid) {
            if (springboard_group_check_group_perm($gid, 'view any order in springboard group', $GLOBALS['user']->uid)) {
              return;
            }
          }
          // If we get here disallow access.
          $query->condition('commerce_order.order_id', array('-1'), 'IN');
        }
      }
    }
  }
}


/**
 * Implements hook_commerce_entity_access_condition_ENTITY_TYPE_alter().
 *
 * Another function for determining view access to orders.
 */
function springboard_group_commerce_entity_access_condition_commerce_order_alter(&$conditions, $context) {

  // @todo is $context[account'] always the current user?
  // if so pass it to check_group_perm
  if (!isset($context['entity'])) {

    $allowed_order_ids = springboard_group_orders_get_allowed_order_ids('view any order in springboard group');
    if (strpos(current_path(), 'admin/commerce/orders') !== FALSE) {
      // Commerce order view provided by commerce module.
      // commerce_order is base table.
      $conditions->condition('commerce_order.order_id', $allowed_order_ids, 'IN');

    }
    else {
      // Donation report page view. Fundraiser is base table.
      $conditions->condition('fundraiser_donation.did', $allowed_order_ids, 'IN');
    }
  }
  else {

    // There's an entity to check.
    $wrapper = entity_metadata_wrapper('commerce_order', $context['entity']);
    if ($wrapper->og_group_ref_commerce_order->raw()) {
      foreach ($wrapper->og_group_ref_commerce_order->raw() as $gid) {
        // If this entity is in a group in which the user has view permission,
        // return true.
        if (springboard_group_check_group_perm($gid, 'view any order in springboard group', $GLOBALS['user']->uid)) {
          return TRUE;
        }
      }
      return FALSE;
    }
  }
}

/**
 * Implements hook_commerce_entity_access_condition_ENTITY_TYPE_alter().
 *
 * For some reason the commerce_customer_profile entity controls view access
 * to the orders overview screen provided by drupal commerce.
 */
function springboard_group_commerce_entity_access_condition_commerce_customer_profile_alter(&$conditions, $context) {
  if (!isset($context['entity'])) {
    $allowed_order_ids = springboard_group_orders_get_allowed_order_ids('view any order in springboard group');
    $conditions->condition('commerce_order.order_id', $allowed_order_ids, 'IN');
  }
  else {

  }
}

function springboard_group_commerce_order_admin_order_view_access($order) {
  if (isset($order)) {
    $group = og_get_entity_groups('commerce_order', $order);
    if (!empty($group)) {
      if (user_access('administer group')
        && user_access('administer orders')
      ) {
        return TRUE;
      }

      $gid = reset($group['node']);
      return springboard_group_check_group_perm($gid, 'edit any order in springboard group', $GLOBALS['user']->uid);
    }
  }
  return commerce_order_admin_order_view_access($order);
}


/**
 * A commerce_payment_transaction_order_access() menu callback wrapper.
 *
 * Manipulate the commerce payment transaction entity permissions.
 */
function springboard_group_commerce_payment_transaction_order_access($op, $order, $account = NULL) {
  global $user;
  $gid = NULL;
  if (empty($account)) {
    $account = clone($user);
  }

  if (isset($order)) {
    $group = og_get_entity_groups('commerce_order', $order);
    if (!empty($group)) {
      if (user_access('administer group')
        && user_access('administer orders')
      ) {
        return TRUE;
      }
      $gid = reset($group['node']);
    }
  }
  switch ($op) {

    case 'view':
      if ($gid) {
        return springboard_group_check_group_perm($gid, 'view payments in springboard group', $account->uid);
      }
    break;

    case 'update':
        return springboard_group_check_group_perm($gid, 'update payments in springboard group', $account->uid);

    case 'delete':
      return springboard_group_check_group_perm($gid, 'delete payments in springboard group', $account->uid);
  }

  return commerce_payment_transaction_order_access($op, $order, $account);
}

/**
 * A commerce_payment_transaction_access() menu callback wrapper.
 *
 * Used in the update and delete paths for commerce payments.
 *
 * @param $op
 * @param $transaction
 * @param null $account
 * @return bool
 */
function springboard_group_commerce_payment_transaction_access($op, $transaction, $account = NULL) {

  if (isset($transaction->order_id)) {
    $order = commerce_order_load($transaction->order_id);
    if (!$order) {
      return FALSE;
    }
  }
  else {
    $order = NULL;
  }
  return springboard_group_commerce_payment_transaction_order_access($op, $order, $account);
}


/**
 * Implements hook_query_TAG_alter().
 *
 * The following is necessary to grant access to the payments overview page.
 *
 * See commerce_payment_query_commerce_payment_transaction_access_alter(), it
 * hard codes a denial of access to anyone who does not have payment view access
 * without providing a sane way to alter that denial. That prevents the access
 * check on payments from ever reaching the
 * commerce_entity_access_condition_ENTITY_ID_alter() hooks implemented in
 * this file.
 */
function springboard_group_query_commerce_payment_transaction_access_alter(QueryAlterableInterface $query) {
  if (!springboard_group_commerce_orders_are_grouped()) {
    return;
  }

  if (strpos(current_path(), 'admin/commerce/orders') !== FALSE) {
    if (is_numeric(arg(3)) && arg(4) == 'payment') {
      $conditions =& $query->conditions();
      // Override the access denial in payments query alter.
      foreach ($conditions as $key => $condition) {
        if (!empty($condition['field']) && is_string($condition['field']) && $condition['field'] == '1 = 0') {
          $order = commerce_order_load(arg(3));
          $wrapper = entity_metadata_wrapper('commerce_order', $order);
          if ($wrapper->og_group_ref_commerce_order->raw()) {
            // Check if the user has access to the order in the orders group.
            foreach ($wrapper->og_group_ref_commerce_order->raw() as $gid) {
              if (springboard_group_check_group_perm($gid, 'view payments in springboard group', $GLOBALS['user']->uid)) {
                // Remove the '1 = 0' condition which denied view access.
                unset($conditions[$key]);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @param $query
 *
 * control access to payment links in views listings?
 */
function springboard_group_query_commerce_payment_transaction_load_multiple_alter(&$query) {
  //$query->condition('base.transaction_id', array('-1'), 'IN');
}

/**
 * Access callback wrapper for views_access() check on donation report.
 */
function springboard_group_donation_report_access($args) {
  if (!user_access('administer group') && $args[0] == 'views_check_perm') {
    return springboard_group_user_has_perm_in_any_group('access donation report in group');
  }
  return views_access($args);
}

/**
 * Implements hook_fundraiser_sustainers_check_access().
 *
 * Check access to the recurring donation admin pages.
 */
function springboard_group_fundraiser_sustainers_check_access($did) {
  if (springboard_group_commerce_orders_are_grouped()) {
    $order = commerce_order_load($did);
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $group = $order_wrapper->og_group_ref_commerce_order->value();
    if (!empty($group[0]->nid)) {
      if (arg(4) == 'recurring'
        && empty(arg(5))
      ) {
        return springboard_group_check_group_perm($group[0]->nid, 'view any order in springboard group', $GLOBALS['user']->uid);
      }

      elseif (arg(4) == 'recurring'
        && empty(arg(5) == 'edit')
      ) {
        return springboard_group_check_group_perm($group[0]->nid, 'edit any order in springboard group', $GLOBALS['user']->uid);
      }
    }

    else {
      if (!user_access('administer group') && !user_access('administrate recurring donations')) {
        return FALSE;
      }
    }
  }
}

/**
 * Utility function.
 *
 * Return the Ids of all orders in a users groups if the user has the
 * correct permission in that group.
 *
 * @todo pass the $uid in so this can be static cached.
 */
function springboard_group_orders_get_allowed_order_ids($perm) {
  // Make sure we have enough values to be able to check access.
  $groups = springboard_group_get_user_group_ids(FALSE);
  $allowed_gids = array(0);
  if (!empty($groups)) {
    foreach ($groups as $gid) {
      // Get the users roles in their groups and check
      // for view access in each group.
      if (springboard_group_check_group_perm($gid, $perm, $GLOBALS['user']->uid)) {
        // Groups in which the user has view access.
        $allowed_gids[] = $gid;
      }
    }
  }

  $allowed_order_ids = array(0);
  $order_ids = db_query('SELECT order_id FROM {commerce_order} co
        INNER JOIN {og_membership} om on co.order_id = om.etid AND om.field_name = :name
        WHERE om.gid IN (:gids)',
    array(':name' => "og_group_ref_commerce_order", ':gids' => $allowed_gids)
  )->fetchAllAssoc('order_id');

  if (!empty($order_ids)) {
    $allowed_order_ids = array_keys($order_ids);
  }
  return $allowed_order_ids;
}
