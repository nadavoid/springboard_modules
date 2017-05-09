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
 * @param $items.
 */
function springboard_group_user__menu_alter(&$items) {
  if (springboard_group_users_are_grouped()) {
    $items['user/%user']['access callback'] = 'springboard_group_user_view_access';
    $items['user/%user/cancel']['access callback'] = 'springboard_group_user_cancel_access';
    $items['user/%user/cancel/confirm/%/%']['access callback'] = 'springboard_group_user_cancel_access';
    $items['user/%user/edit']['access callback'] = 'springboard_group_user_edit_access';
  }
}

/**
 * User view access callback.
 *
 * @param $account
 *   Can either be a full user object or a $uid.
 */
function springboard_group_user_view_access($account) {
  $uid = is_object($account) ? $account->uid : (int) $account;

  // Never allow access to view the anonymous user account.
  if ($uid) {
    // Admins can view all, users can view own profiles at all times.
    if ($GLOBALS['user']->uid == $uid || user_access('administer users')) {
      return TRUE;
    }
    elseif (user_access('access user profiles')) {
      // At this point, load the complete account object.
      if (!is_object($account)) {
        $account = user_load($uid);
      }
      return (is_object($account) && $account->status);
    }
  }
  return FALSE;
}

/**
 * Access callback for user account editing.
 */
function springboard_group_user_edit_access($account) {
  return (($GLOBALS['user']->uid == $account->uid) || user_access('administer users')) && $account->uid > 0;
}

/**
 * Menu access callback; limit access to account cancellation pages.
 *
 * Limit access to users with the 'cancel account' permission or administrative
 * users, and prevent the anonymous user from cancelling the account.
 */
function springboard_group_user_cancel_access($account) {
  return ((($GLOBALS['user']->uid == $account->uid) && user_access('cancel account')) || user_access('administer users')) && $account->uid > 0;
}


/**
 * Implements hook_form_FORM_ID_alter().
 */
function springboard_group_form_user_profile_form_alter(&$form, &$form_state) {
  // We don't need to see the group membership field on the user edit page.
  // It is large and obstructive and not very useful.
  $profile_edit_form = arg(3);
  if (empty($profile_edit_form)) {
    $form['og_user_node']['#access'] = FALSE;
    $account = user_load(arg(1));
    $groups = og_get_entity_groups('user', $account);
    $titles = [];
    if (!empty($groups['node'])) {
      foreach ($groups['node'] as $group_id) {
        $group = node_load($group_id);
        $titles[] = check_plain($group->title);
      }
      $form['user_group_display']['#markup'] = theme('item_list', array(
        'items' => $titles,
        'title' => 'Group Membership',
      ));
    }
  }

  // No groups field, do not proceed.
  if (!springboard_group_users_are_grouped() || !empty($profile_edit_form)) {
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
    $language = $form['og_group_ref_user']['#language'];
    $form['og_group_ref_user']['#title'] = t('Contact owners');
    $form['og_group_ref_user'][$language][0]['default']['#description'] = t('The groups this contact is owned by.');
    $form['og_group_ref_user'][$language][0]['default']['#ajax'] = array(
      'callback' => 'springboard_group_og_group_field_ajax_callback',
    );

    // Set up the validation modal window parameters.
    springboard_group_prepare_validation_modal($form, $form_state);

    // Add the validation handlers to the form.
    springboard_group_load_ajax_validation_handlers($form, $form_state);
  }
}
