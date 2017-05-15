<?php

/**
 * @file
 * Permissions and access related functions for group adovacy.
 */

/**
 * Implements hook_springboard_target_target_access().
 */
function springboard_group_springboard_target_target_access($op, $item_id) {
  global $user;

  if (user_access('administer group')) {
    return user_access($op);
  }
  else {

    $groups = og_get_groups_by_user();
    $context = springboard_group_get_group_context();

    // First, check if access should be denied due to mismatched affiliate id
    // or other issues.
    switch ($op) {

      case 'edit groups':
      case 'delete groups':
        if (!empty($item_id)) {
          // Test for target ownership by group.
          $api_call = springboard_advocacy_api_call();
          $response = $api_call->invokeClientMethod('getTargetGroup', $item_id);
          if (empty($response->data)) {
            return FALSE;
          }
          if ($response->data->affiliate_id != $context) {
            return FALSE;
          }
          if (!in_array($response->data->affiliate_id, $groups['node'])) {
            return FALSE;
          }
        }
        break;

      case 'edit targets':
      case 'delete targets':
        if (!empty($item_id)) {
          $api_call = springboard_advocacy_api_call();
          $response = $api_call->invokeClientMethod('getCustomTarget', $item_id);
          if (empty($response->data)) {
            return FALSE;
          }
          if ($response->data->affiliate_id != $context) {
            return FALSE;
          }
          if (!in_array($response->data->affiliate_id, $groups['node'])) {
            return FALSE;
          }
        }
        break;
    }
  }

  // If we get this far, check for in-group permissions.
  if (!user_access($op)) {
    $account = $user;
    $group_perms = array_keys(array_filter(springboard_group_advocacy_get_advocacy_group_perms($account)));
    return in_array($op, $group_perms);
  }
}

/**
 * Implements hook_springboard_advocacy_user_can_create_advocacy_type_alter().
 *
 * Shows advocacy menu if true.
 */
function springboard_group_advocacy_springboard_advocacy_user_can_create_advocacy_type() {
    return springboard_group_advocacy_user_can_create_advocacy_type();
}

/**
 * Implements hook_springboard_advocacy_user_can_create_advocacy_type_alter().
 *
 * Shows advocacy menu if true.
 */
function springboard_group_advocacy_springboard_advocacy_user_can_edit_any_advocacy_type() {
    return springboard_group_advocacy_user_can_edit_advocacy_type();
}

/**
 * Check for group-based advocacy content type permissions.
 *
 * Cycle through all of a users group memberships and group roles and check if
 * the user can create advocacy content in any of them.
 *
 * @param object $account
 *   A user account object.
 *
 * @return bool
 *   TRUE if the user can create advocacy content in any group.
 */
function springboard_group_advocacy_user_can_create_advocacy_type($account = NULL) {
  global $user;
  if (empty($account)) {
    $account = $user;
  }
  $groups = og_get_entity_groups('user', $account);
  $access = FALSE;
  if (!empty($groups['node'])) {
    foreach ($groups['node'] as $gid) {
      if ($roles = og_get_user_roles('node', $gid)) {
        $perms = og_role_permissions($roles);
        $role_permissions = [];
        foreach ($perms as $perm) {
          $role_permissions = array_merge($role_permissions, array_keys(array_filter($perm)));
        }
        $content_perms = array(
          'create sba_message_action content',
          'create sba_social_action content',
          'create springboard_petition content',
        );
        foreach ($content_perms as $content_perm) {
          if (in_array($content_perm, $role_permissions)) {
            $access = TRUE;
          }
        }
      }
    }
  }
  return $access;
}

function springboard_group_advocacy_user_can_edit_advocacy_type($account = NULL) {
  global $user;
  if (empty($account)) {
    $account = $user;
  }
  $groups = og_get_entity_groups('user', $account);
  $access = FALSE;
  if (!empty($groups['node'])) {
    foreach ($groups['node'] as $gid) {
      if ($roles = og_get_user_roles('node', $gid)) {
        $perms = og_role_permissions($roles);
        $role_permissions = [];
        foreach ($perms as $perm) {
          $role_permissions = array_merge($role_permissions, array_keys(array_filter($perm)));
        }
        $content_perms = array(
          'update any sba_message_action content',
          'update any sba_social_action content',
          'update any springboard_petition content',
        );
        foreach ($content_perms as $content_perm) {
          if (in_array($content_perm, $role_permissions)) {
            $access = TRUE;
          }
        }
      }
    }
  }
  return $access;
}
