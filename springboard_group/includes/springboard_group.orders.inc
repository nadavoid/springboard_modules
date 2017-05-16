<?php
/**
 * @file
 * Orders definitions and hooks for Springboard Group.
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
 * Permission Checker.
 *
 * @param $gid
 * @param $permission
 *
 * @return bool
 */
function springboard_group_commerce_order_check_group_perms($gid, $permission) {
  //@todo cache. send uid?
  if ($roles = og_get_user_roles('node', $gid)) {
    $perms = og_role_permissions($roles);
    foreach ($perms as $perm_array) {
      if (in_array($permission, $perm_array)) {
        return TRUE;
      }
    }
  }
  return FALSE;
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
    if(1) {
      $groups = og_get_entity_groups('commerce_order', $form['#entity']);
      $form['og_group_ref_commerce_order']['#access'] = FALSE;
      $titles = [];
      if (!empty($groups['node'])) {
        foreach ($groups['node'] as $group_id) {
          $group = node_load($group_id);
          $titles[] = check_plain($group->title);
        }
        $form['order_group_display_holder'] =  array(
          '#type' => 'fieldset',
          '#title' => t('Group Ownership')
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
 * @param $form
 * @param $form_state
 * @param null $callback
 */
function springboard_group_commerce_order_form__validate(&$form, &$form_state, $callback = NULL) {
  if ($callback != 'springboard_group_og_group_field_ajax_callback') {
    return;
  }
  // @todo, if any.
}


/**
 * Implements hook_fundraiser_donation_create().
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
 * Implements hook_commerce_order_presave().
 *
 * Update group membership on commerce orders when admin edits order and
 * changes group affiliation.
 */
function springboard_group_commerce_order_presave($order) {

  if (!empty($order->og_group_ref_commerce_order) && !empty($order->original)) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $original_order_wrapper = entity_metadata_wrapper('commerce_order', $order->original);
    $new_group = $order_wrapper->og_group_ref_commerce_order->value();
    $old_group = $original_order_wrapper->og_group_ref_commerce_order->value();

    $new_group_gid = $new_group[0]->nid;
    $old_group_gid = $old_group[0]->nid;
    if ($new_group_gid != $old_group_gid) {
      // @todo grab associated sustainer orders and update them too
      // or this should never happen because group ownership should not be
      // editable.
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
          return springboard_group_commerce_order_check_group_perms($gid, 'edit any group commerce_order entity');
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
 * Limit view access to commerce orders by group ownsership.
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

      // Get the users groups.
      $groups = springboard_group_get_user_group_ids(FALSE);

      $allowed_gids = array(0);

      if (!empty($groups)) {
        foreach ($groups as $gid) {
          // Get the users roles in their groups and check
          // for view access in each group.
          if (springboard_group_commerce_order_check_group_perms($gid, 'view any group commerce_order entity')) {
            // Groups in which the user has view access.
            $allowed_gids[] = $gid;
          }
        }
      }
      $allowed_gids = implode(',', $allowed_gids);

      // If this is a listing, there is no order Id to check. We have to load
      // all orders in a group and check against the array.
      // @todo: really?
      if (empty($id)) {
        $allowed_order_ids = array(0);
        $order_ids = db_query('SELECT order_id FROM {commerce_order} co
        INNER JOIN {og_membership} om on co.order_id = om.etid AND om.field_name = :name
        WHERE om.gid IN (:gids)',
          array(':name' => "og_group_ref_commerce_order", ':gids' => $allowed_gids)
        )->fetchAllAssoc('order_id');

        if (!empty($order_ids)) {
          $allowed_order_ids = array_keys($order_ids);
        }

        $query->condition('commerce_order.order_id', $allowed_order_ids, 'IN');

      }
      else {
        // There's an order id in the query. Check to see if it's in the groups.
        $is_in_allowed_group = db_query('SELECT etid FROM {og_membership} om
        WHERE om.gid IN (' . $allowed_gids . ')
        AND om.field_name = :name
        AND om.etid = :id
      ', array(
          ':name' => "og_group_ref_commerce_order",
          ':id' => $id
        ))->fetchField();

        if (empty($is_in_allowed_group)) {
          $query->condition('commerce_order.order_id', array('-1'), 'IN');
        }
      }
    }
  }
}


/**
 * Menu alter helper.
 *
 * The order payment paths are controlled by access functions which are not
 * alterable in the way we need, so we have to wrap the original access
 * functions in a custom access function.
 *
 * @param $items
 */
function springboard_group__commerce_order__menu_alter(&$items) {
  if (springboard_group_commerce_orders_are_grouped()) {
    $items['admin/commerce/orders/%commerce_order/payment']['access callback'] = 'springboard_group_commerce_payment_transaction_order_access';
    $items['admin/commerce/orders/%commerce_order/payment/%commerce_payment_transaction']['access callback'] = 'springboard_group_commerce_payment_transaction_access';
    $items['admin/commerce/orders/%commerce_order/payment/%commerce_payment_transaction/edit']['access callback'] = 'springboard_group_commerce_payment_transaction_access';
    $items['admin/commerce/orders/%commerce_order/payment/%commerce_payment_transaction/delete']['access callback'] = 'springboard_group_commerce_payment_transaction_access';

  }
}

/**
 * A commerce_payment_transaction_order_access() wrapper.
 *
 * Manipulate the commerce payment transaction entity permissions.
 */
function springboard_group_commerce_payment_transaction_order_access($op, $order, $account = NULL) {
  global $user;

  if (empty($account)) {
    $account = clone($user);
  }

  //@todo update to check group perms.
  switch ($op) {
    // Viewing payment transactions.
    case 'view':
      if (user_access('view payments', $account)) {
        if (commerce_order_access('view', $order, $account)) {
          return TRUE;
        }
        else {
          return FALSE;
        }
      }
      break;

    case 'update':
      if (user_access('update payments', $account)) {
        if (commerce_order_access('view', $order, $account)) {
          return TRUE;
        }
        else {
          return FALSE;
        }
      }
      break;

    case 'delete':
      if (user_access('delete payments', $account)) {
        if (commerce_order_access('update', $order, $account)) {
          return TRUE;
        }
        else {
          return FALSE;
        }
      }
      break;
  }

  return commerce_payment_transaction_order_access($op, $order, $account);
}

/**
 * A commerce_payment_transaction_access() wrapper.
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
 * @param \QueryAlterableInterface $query
 */
function springboard_group_query_commerce_payment_transaction_access_alter(QueryAlterableInterface $query) {
  // Read the meta-data from the query.
  if (!$account = $query->getMetaData('account')) {
    global $user;
    $account = $user;
  }

  if (user_access('view payments', $account)) {

    //@todo not sure we need this.
    //$query->where('1 = 0');

  }
}

/**
 * @param $query
 */
function springboard_group_query_commerce_payment_transaction_load_multiple_alter(&$query) {
  //$query->condition('base.transaction_id', array('-1'), 'IN');
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
        return springboard_group_commerce_order_check_group_perms($group[0]->nid, 'view any group commerce_order entity');
      }

      elseif (arg(4) == 'recurring'
        && empty(arg(5) == 'edit')
      ) {
        return springboard_group_commerce_order_check_group_perms($group[0]->nid, 'edit any group commerce_order entity');
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
 * Add the order-related permissions.
 *
 * Sub-hook to springboard_group_og_permissions() hook.
 */
function springboard_group__commerce_order__og_permissions(&$items) {
  $orders_grouped = og_get_group_audience_fields('commerce_order', 'commerce_order', 'node', 'springboard_group');
  if (!empty($orders_grouped)) {

    $items['create orders in springboard group'] = array(
      'title' => t('Create  orders in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );
    $items['edit any order in springboard group'] = array(
      'title' => t('Edit any order in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['administer orders in springboard group'] = array(
      'title' => t('Administer orders in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['view any order in springboard group '] = array(
      'title' => t('View any order in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['view payments in springboard group'] = array(
      'title' => t('View payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['create payments in springboard group'] = array(
      'title' => t('Create payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['delete payments in springboard group'] = array(
      'title' => t('Delete payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );
    $items['update payments in springboard group'] = array(
      'title' => t('Update payments in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['edit any card data in springboard group'] = array(
      'title' => t('Edit any card data in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['view any card data in springboard group'] = array(
      'title' => t('View any card data in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['create card data  in springboard group'] = array(
      'title' => t('Create card data  in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['delete any card data in springboard group'] = array(
      'title' => t('Delete any card data in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );
    $items['submit springboard group donations offline'] = array(
      'title' => t('Submit donations offline in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

    $items['cancel future donations in springboard group'] = array(
      'title' => t('Cancel future donations in in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );
    $items['fastforward recurring donations in springboard group'] = array(
      'title' => t('Fast-forward recurring donations in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
    );

//  $items['administrate recurring donations'] = array(
//    'title' => t('Administer recurring donations'),
//    'description' => '',
//    'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
//    'default role' => array(),
//  );
  }
}