<?php
/**
 * @file
 * Orders definitions and hooks for Springboard Group.
 */

function springboard_group_users_are_grouped($fields = FALSE) {
  $group_enabled_field = &drupal_static(__FUNCTION__);
  if (!isset($group_enabled_field)) {
    $group_enabled_field = og_get_group_audience_fields('user', 'user', 'node', 'springboard_group');
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
function springboard_group_user_check_group_perms($gid, $permission) {
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
 * @param $form
 * @param $form_state
 * @param null $callback
 */
function springboard_group_user_form__validate(&$form, &$form_state, $callback = NULL) {
  if ($callback != 'springboard_group_og_group_field_ajax_callback') {
    return;
  }
  // @todo, if any.
}


/**
 * Implements hook_webform_user_submission_insert().
 */
function springboard_group_webform_user_submission_insert($node, $submission, $account) {

  if (springboard_group_users_are_grouped()) {

    $node_wrapper = entity_metadata_wrapper('node', $node);
    $group_field_name = 'og_group_ref_' . substr($node->type, 0, 19);
    if (!empty($node_wrapper->{$group_field_name})) {
      $node_groups = $node_wrapper->{$group_field_name}->value();
      $gids = array();
      foreach ($node_groups as $id => $group) {
        if (!empty($group->nid)) {
          $gids[] = $group->nid;
        }
      }
      if (!empty($gids)) {
        $user = user_load($account->uid);
        $user_wrapper = entity_metadata_wrapper('user', $user);
        $current_groups = $user_wrapper->og_group_ref_user->value();
        $cgids = array();
        foreach ($current_groups as $cgroup) {
          $cgids[] = $cgroup->nid;
        }
        $merged = array_unique(array_merge($gids, $cgids));
        $user_wrapper->og_group_ref_user->set($merged);
        $user_wrapper->save();
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
function springboard_group_user__menu_alter(&$items) {
  if (springboard_group_users_are_grouped()) {

  }
}


/**
 * Implements hook_form_FORM_ID_alter().
 */
function springboard_group_form_user_profile_form_alter(&$form, &$form_state) {
  // We don't need to see the group membership field on the user edit page.
  // It is large and obstructive and not very useful.
  $form['og_user_node']['#access'] = FALSE;
  $account = user_load(arg(1));
  $groups = og_get_entity_groups('user', $account);
  $titles = [];
  if (!empty($groups['node'])) {
    foreach ($groups['node'] as $group_id) {
      $group = node_load($group_id);
      $titles[] = check_plain($group->title);
    }
    $form['user_group_display']['#markup'] = theme('item_list', array('items' => $titles, 'title' => 'Springboard Groups Membership'));
  }

  // No groups field, do not proceed.
  if (!springboard_group_users_are_grouped()) {
    return;
  }

  // Convert the multiple select widget to a single selection widget.
  // See springboard_group_field_widget_form_alter() and
  // springboard_group_set_single_selector() for details about
  // why this is necessary.
  $fields = springboard_group_users_are_grouped(TRUE);

  if (!empty($fields)) {
    springboard_group_set_single_selector($form, $fields);

    // Add ajax callback to the groups field so we can validate newly
    // selected groups.
    $language = $form['og_group_ref_' . 'user']['#language'];
    $form['og_group_ref_' . 'user']['#title'] =  t('Springboard groups ownership');
    $form['og_group_ref_' . 'user'][$language][0]['default']['#ajax'] = array(
      'callback' => 'springboard_group_og_group_field_ajax_callback',
    );

    // Set up the validation modal window parameters.
    springboard_group_prepare_validation_modal($form, $form_state);

    // Add the validation handlers to the form.
    springboard_group_load_ajax_validation_handlers($form, $form_state);
  }
}
