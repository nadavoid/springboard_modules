<?php
/**
 * @file
 * Orders definitions and hooks for Springboard Group.
 */
function springboard_group_users_are_grouped($fields = FALSE) {
  $group_enabled_field = &drupal_static(__FUNCTION__ . $fields);
  if (!isset($group_enabled_field)) {
    $group_enabled_field = field_info_instance('user', 'user_sb_group_ref', 'user');
  }
  if (!$fields) {
    return (bool) $group_enabled_field;
  }
  else {
    return $group_enabled_field;
  }
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
        $current_groups = $user_wrapper->user_sb_group_ref->value();
        $cgids = array();
        foreach ($current_groups as $cgroup) {
          $cgids[] = $cgroup->nid;
        }
        $merged = array_unique(array_merge($gids, $cgids));
        $user_wrapper->user_sb_group_ref->set($merged);
        $user_wrapper->save();
      }
    }
  }
}

/**
 * Menu alter helper.
 *
 * Replace user-related access callbacks with their group-wrapped versions.
 *
 * @param $items.
 */
function springboard_group__user__menu_alter(&$items) {
  if (springboard_group_users_are_grouped()) {
    $items['user/%user']['access callback'] = 'springboard_group_user_view_access';
    $items['user/%user/cancel']['access callback'] = 'springboard_group_user_cancel_access';
    $items['user/%user/cancel/confirm/%/%']['access callback'] = 'springboard_group_user_cancel_access';
    $items['user/%user/edit']['access callback'] = 'springboard_group_user_edit_access';
    $items['user/%user/recurring_overview']['access callback'] = 'springboard_group_user_edit_access';
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
      //@todo need to check if users are grouped?
      $contact_gids = springboard_group_get_contact_group_gids($uid);
      if (!empty($contact_gids)) {
        $groups = springboard_group_get_user_group_ids(FALSE);
        foreach ($contact_gids as $gid) {
          if (in_array($gid, $groups) && springboard_group_check_group_perm($gid, 'access user profiles in springboard group', $GLOBALS['user']->uid)) {
            return TRUE;
          }
          if (user_access('administer groups') && user_access('administer users')) {

            return TRUE;
          }
        }
//      $strict = variable_get('node_access_strict', 0);
//      if ($strict) {
//        // If the user is grouped at all, return false, even if the admin has
//        // administer permissions but not group admin permissions?
//         return FALSE;
//      }
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
      $editors_groups = springboard_group_get_user_group_ids(FALSE);
      foreach ($contact_gids as $gid) {
        if (in_array($gid, $editors_groups) && springboard_group_check_group_perm($gid, 'administer users in springboard group', $GLOBALS['user']->uid)) {
          return TRUE;
        }
        if (user_access('administer groups') && user_access('administer users')) {
          return TRUE;
        }
      }
//      $strict = variable_get('node_access_strict', 0);
//      if ($strict) {
//        // If the user is grouped at all, return false, even if the admin has
//        // administer permissions but not group admin permissions?
//         return FALSE;
//      }
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
        if (in_array($gid, $groups) && springboard_group_check_group_perm($gid, 'administer users in springboard group', $GLOBALS['user']->uid)) {
          return TRUE;
        }
        if (user_access('administer groups') && user_access('administer users')) {
          return TRUE;
        }
      }
//      $strict = variable_get('node_access_strict', 0);
//      if ($strict) {
//        // If the user is grouped at all, return false, even if the admin has
//        // administer permissions but not group admin permissions?
//         return FALSE;
//      }
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

  $fields = springboard_group_users_are_grouped();
  if (!empty($fields)) {
    // Show a read-only contact ownership field.
    $language = $form['user_sb_group_ref']['#language'];
    $form['user_sb_group_ref'][$language]['#access'] = FALSE;
    if (!empty($form['user_sb_group_ref'][$language]['#default_value'])) {
      $titles = array();
      foreach ($form['user_sb_group_ref'][$language]['#default_value'] as $group_id) {
        $group = node_load($group_id);
        $titles[] = check_plain($group->title);
      }
      $form['contact_group_display']['#markup'] = theme('item_list', array(
        'items' => $titles,
        'title' => 'Contact Ownership',
      ));
    }
  }
}


function springboard_group_get_contact_group_gids($uid) {
  $contact_gids = &drupal_static(__FUNCTION__ . $uid);
  if (!isset($contact_gids)) {
    $contact = user_load($uid);
    $contact_gids = array();
    if (!empty($contact->user_sb_group_ref)) {
      $contact_wrapper = entity_metadata_wrapper('user', $contact);
      $groups = $contact_wrapper->user_sb_group_ref->value();
      foreach ($groups as $group) {
        $contact_gids[] = $group->nid;
      }
    }
  }
  return $contact_gids;
}

function springboard_group__user__og_permissions(&$items) {
  if (springboard_group_users_are_grouped()) {

    $items['cancel account in springboard group'] = array(
      'title' => t('Cancel accounts in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'user',
    );
    $items['administer users in springboard group'] = array(
      'title' => t('Administer users in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'user',
    );

    $items['access user profiles in springboard group'] = array(
      'title' => t('View user profiles in Springboard Group'),
      'description' => '',
      'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
      'default role' => array(),
      'module' => 'user',
    );

  }
}