<?php
/**
 * @file
 * Webform functions for springboard groups.
 *
 * @see https://www.drupal.org/node/1946432.
 */

/**
 * Implements hook_webform_submission_access().
 *
 * @see webform_submission_access()
 */
function springboard_group_webform_submission_access($node, $submission, $op = 'view', $account = NULL) {
  global $user;
  $account = isset($account) ? $account : $user;

  // Ensure a full node object, as Views handlers may not load the entire node.
  if (empty($node->vid) && !empty($node->nid)) {
    $node = node_load($node->nid);
  }

  $edit_submission = FALSE;
  $delete_submission = FALSE;
  $access_all = FALSE;
  if (og_is_group_content_type('node', $node->type) && $groups = og_get_entity_groups('node', $node)) {

    // Global group admins.
    if (user_access('administer group')) {
      if (user_access('access all webform results')) {
        $access_all = TRUE;
      }
      if (user_access('delete all webform submissions')) {
        $delete_submission = TRUE;
      }
      if (user_access('edit all webform submissions')) {
        $edit_submission = TRUE;
      }
    }

    // Springboard Group members.
    foreach ($groups as $group_type => $gids) {
      foreach ($gids as $gid) {
        // Only need access in one of the associated groups.
        if (!$access_all) {
          // Determine if the user has access to all results and submissions.
          if (og_user_access($group_type, $gid, 'access all webform results', $account)) {
            $access_all = TRUE;
          }
          if (og_user_access($group_type, $gid, 'access all ' . $node->type . ' webform results', $account)) {
            $access_all = TRUE;
          }
        }

        if (!$edit_submission && $access_all && $op == 'edit') {
          if (og_user_access($group_type, $gid, 'edit all webform submissions', $account)) {
            $edit_submission = TRUE;
          }
          if (og_user_access($group_type, $gid, 'edit all ' . $node->type . ' webform submissions', $account)) {
            $edit_submission = TRUE;
          }
        }

        if (!$delete_submission && $access_all && $op == 'delete') {
          if (og_user_access($group_type, $gid, 'delete all webform submissions', $account)) {
            $delete_submission = TRUE;
          }
          if (og_user_access($group_type, $gid, 'delete all ' . $node->type . ' webform submissions', $account)) {
            $delete_submission = TRUE;
          }
        }
      }
    }
  }
  else {
    // If this webform is not in a group, don't affect access.
    return;
  }

  switch ($op) {
    case 'view':
      return $access_all;

    case 'list':
      return $access_all && ($account->uid || isset($_SESSION['webform_submission']));

    case 'edit':
      return $edit_submission;

    case 'delete':
      return $delete_submission;
  }
}

/**
 * Implements hook_webform_results_access().
 */
function springboard_group_webform_results_access($node, $account = NULL) {
  if (og_is_group_content_type('node', $node->type) && $groups = og_get_entity_groups('node', $node)) {
    foreach ($groups as $group_type => $gids) {
      foreach ($gids as $gid) {
        if ((user_access('administer group') && user_access('access all webform results'))
          || og_user_access($group_type, $gid, 'access all webform results', $account)
          || og_user_access($group_type, $gid, 'access all ' . $node->type . ' webform results', $account)
        ) {
          return TRUE;
        }
      }
    }
  }
  else {
    // If this webform is not in a group, don't affect access.
    return;
  }
}

/**
 * Implements hook_webform_results_clear_access().
 */
function springboard_group_webform_results_clear_access($node, $account = NULL) {
  if (og_is_group_content_type('node', $node->type) && $groups = og_get_entity_groups('node', $node)) {
    foreach ($groups as $group_type => $gids) {
      foreach ($gids as $gid) {
        if ((user_access('administer group') && user_access('delete all webform submissions'))
          || og_user_access($group_type, $gid, 'delete all webform submissions', $account)
          || og_user_access($group_type, $gid, 'delete all ' .$node->type . ' webform submissions', $account)) {
          return springboard_group_webform_results_access($node, $account);
        }
      }
    }
  }
  else {
    return;
  }
}
