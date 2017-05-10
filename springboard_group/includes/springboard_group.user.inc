<?php
/**
 * @file
 * Orders definitions and hooks for Springboard Group.
 */

function springboard_group_users_are_grouped($fields = FALSE) {
  $group_enabled_field = &drupal_static(__FUNCTION__ . $fields);
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
 * Permission checker for current user.
 *
 * Takes a group ID and a permission string and determines whether the current
 * user has that permission in the group.
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
function springboard_group__user__menu_alter(&$items) {
  if (springboard_group_users_are_grouped()) {
    $items['user/%user']['access callback'] = 'springboard_group_user_view_access';
    $items['user/%user/cancel']['access callback'] = 'springboard_group_user_cancel_access';
    $items['user/%user/cancel/confirm/%/%']['access callback'] = 'springboard_group_user_cancel_access';
    $items['user/%user/edit']['access callback'] = 'springboard_group_user_edit_access';
    $items['user/%user/recurring_overview'] = 'springboard_group_user_edit_access';
    $items['user/%user/donation-history']['access callback'] = 'springboard_group_user_edit_access';
    $items['user/%user/donation-history/summary/%']['access callback'] = 'springboard_group_user_edit_access';
    $items['user/%user/donation-history/receipt/%']['access callback'] = 'springboard_group_user_edit_access';
  }
}

/**
 * User view access callback.
 */
function springboard_group_user_view_access($account) {
  $uid = is_object($account) ? $account->uid : (int) $account;
  if ($GLOBALS['user']->uid != $uid) {
    if ($uid) {
      $contact_gids = springboard_group_get_contact_group_gids($uid);
      if (!empty($contact_gids)) {
        $groups = springboard_group_get_user_group_ids(FALSE);
        foreach ($contact_gids as $gid) {
          if (in_array($gid, $groups) && springboard_group_user_check_group_perms($gid, 'view group contacts')) {
            return TRUE;
          }
          if (user_access('administer groups') && user_access('administer users')) {
            return TRUE;
          }
        }
        // If the user is grouped at all, return false, even if the admin has
        // administer permissions but not group admin permissions?
        // return FALSE;
      }
    }
  }
  // Use the normal access check if we get here.
  return user_view_access($account);
}

/**
 * Access callback for user account editing.
 */
function springboard_group_user_edit_access($account) {
  if ($GLOBALS['user']->uid != $account->uid) {
    $contact_gids = springboard_group_get_contact_group_gids($account->uid);
    if (!empty($contact_gids)) {
      $groups = springboard_group_get_user_group_ids(FALSE);
      foreach ($contact_gids as $gid) {
        if (in_array($gid, $groups) && springboard_group_user_check_group_perms($gid, 'manage group contacts')) {
          return TRUE;
        }
        if (user_access('administer groups') && user_access('administer users')) {
          return TRUE;
        }
      }
      // If the user is grouped at all, return false, even if the admin has
      // administer permissions but not group admin permissions?
      // return FALSE;
    }
  }
  // Use the normal access check if we get here.
  return user_edit_access($account);
}

/**
 * Menu access callback; limit access to account cancellation pages.
 */
function springboard_group_user_cancel_access($account) {
  if ($GLOBALS['user']->uid != $account->uid) {
    $contact_gids = springboard_group_get_contact_group_gids($account->uid);
    if (!empty($contact_gids)) {
      $groups = springboard_group_get_user_group_ids(FALSE);
      foreach ($contact_gids as $gid) {
        if (in_array($gid, $groups) && springboard_group_user_check_group_perms($gid, 'manage group contacts')) {
          return TRUE;
        }
        if (user_access('administer groups') && user_access('administer users')) {
          return TRUE;
        }
      }
      // If the user is grouped at all, return false, even if the admin has
      // administer permissions but not group admin permissions?
      // return FALSE;
    }
  }
  // Use the normal access check if we get here.
  return user_cancel_access($account);
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


function springboard_group_get_contact_group_gids($uid) {
  $contact_gids = &drupal_static(__FUNCTION__ . $uid);
  if (!isset($contact_gids)) {
    $contact = user_load($uid);
    $contact_gids = array();
    if (!empty($contact->og_group_ref_user)) {
      $contact_wrapper = entity_metadata_wrapper('user', $contact);
      $groups = $contact_wrapper->og_group_ref_user->value();
      foreach ($groups as $group) {
        $contact_gids[] = $group->nid;
      }
    }
  }
  return $contact_gids;
}