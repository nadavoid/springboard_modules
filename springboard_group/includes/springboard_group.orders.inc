<?php
/**
 * @file
 * Orders definitions and hooks for Springboard Group.
 */


/**
 * Add the group widget to the commerce order form.
 */
function springboard_group_form_commerce_order_ui_order_form_alter(&$form, &$form_state) {

  // Check that this entity has a groups field.
  $childs = element_children($form);
  $grouped = FALSE;
  foreach ($childs as $child) {
    if (strpos($child, 'og_group_ref') !== FALSE) {
      $grouped = TRUE;
    }
  }
  // No groups field, do not proceed.
  if (!$grouped) {
    return;
  }

  // Convert the multiple select widget to a single selection widget.
  // See springboard_group_field_widget_form_alter() and
  // springboard_group_set_single_selector() for details about
  // why this is necessary.
  $fields = og_get_group_audience_fields('commerce_order', 'commerce_order', 'node', 'springboard_group');

  if (!empty($fields)) {
    springboard_group_set_single_selector($form, $fields);

    // Add ajax callback to the groups field so we can validate newly
    // selected groups.
    $language = $form['og_group_ref_' . 'commerce_order']['#language'];
    $form['og_group_ref_' . 'commerce_order'][$language][0]['default']['#ajax'] = array(
      'callback' => 'springboard_group_og_group_field_ajax_callback',
    );

    // Set up the validation modal window parameters.
    springboard_group_prepare_validation_modal($form, $form_state);

    // Add the validation handlers to the form.
    springboard_group_load_ajax_validation_handlers($form, $form_state);
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
function springboard_group_fundraiser_donation_create($donation) {
  // Get basic information from the donation node.
  $fields = og_get_group_audience_fields('commerce_order', 'commerce_order', 'node', 'springboard_group');
  if (!empty($fields)) {
    $node_wrapper = entity_metadata_wrapper('node', $donation->node);
    $group_field_name = 'og_group_ref_' . substr($donation->node->type, 0, 19);
    $node_group = $node_wrapper->{$group_field_name}->value();
    $node_group_id = $node_group[0]->nid;
    // Get the original order.
    $order = commerce_order_load($donation->did);
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $order_wrapper->og_group_ref_commerce_order->set(array('target_id' => $node_group_id));
    $order_wrapper->save();
  }
}

/**
 * @param $order
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
      // grab associated sustainer orders and update them too
    }
  }
}

/**
 * @param $op
 * @param $entity
 * @param $account
 * @param $entity_type
 * @return bool
 */
function springboard_group_commerce_entity_access($op, $entity, $account, $entity_type) {
  $fields = og_get_group_audience_fields('commerce_order', 'commerce_order', 'node', 'springboard_group');
  if (!empty($fields)) {
    // Check for group permissions.
    if (isset($entity)) {
      if (($op == 'update' || $op == 'delete')) {
        if ($entity_type == 'commerce_order') {
          $group = og_get_entity_groups('commerce_order', $entity);
        }
        elseif ($entity_type == 'commerce_payment') {
          // @todo probably not needed due to commerce permissions cascading.
        }
        if (!empty($group)) {
          $access = FALSE;
          // Check for global permissions.
          if (user_access('administer groups')
          ) {
            return TRUE;
          }
          $gid = reset($group['node']);
          if ($roles = og_get_user_roles('node', $gid)) {
            $perms = og_role_permissions($roles);
            foreach ($perms as $perm_array) {
              if (in_array('manage group orders', $perm_array)) {
                return TRUE;
              }
            }
          }
          return $access;
        }
      }
    }
  }
}

/**
 * limit view access
 */
function springboard_group_query_commerce_order_access_alter(&$query) {
  if (!user_access('administer group')) {
    $fields = og_get_group_audience_fields('commerce_order', 'commerce_order', 'node', 'springboard_group');
    if (!empty($fields)) {

      foreach ($query->conditions() as $condition) {
        if (!empty($condition['field']) && is_string($condition['field']) && $condition['field'] == 'order_id') {
          $id = $condition['value'];
        }
      }

      $groups = og_get_groups_by_user(NULL, 'node');
      $allowed_gids = array(0);

      if (!empty($groups)) {
        foreach ($groups as $gid) {
          if ($roles = og_get_user_roles('node', $gid)) {
            $perms = og_role_permissions($roles);
            foreach ($perms as $perm_array) {
              if (in_array('view group orders', $perm_array)) {
                $allowed_gids[] = $gid;
              }
            }
          }
        }
      }

      $allowed_gids = implode(',', $allowed_gids);

      if (empty($id)) {
        $allowed_order_ids = array(0);
        $order_ids = db_query('SELECT order_id FROM {commerce_order} co
        INNER JOIN {og_membership} om on co.order_id = om.etid AND om.field_name = :name
        WHERE om.gid IN (' . $allowed_gids . ')',
          array(':name' => "og_group_ref_commerce_order")
        )->fetchAllAssoc('order_id');

        if (!empty($order_ids)) {
          $allowed_order_ids = array_keys($order_ids);
        }
        $query->condition('commerce_order.order_id', $allowed_order_ids, 'IN');

      }
      else {
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
