<?php
/**
 * @file
 * Menu callback functions.
 */

/**
 * Bulk add users to a group.
 *
 * @param string $group_type
 *   Always node.
 * @param int $group_id
 *   The group ID.
 *
 * @return mixed
 *   An empty string if permissions are lacking, the bulk add view
 *   if permissions exist.
 */
function springboard_group_bulk_add_page($group_type, $group_id) {
  $content = '';
  if ($roles = og_get_user_roles('node', $group_id)) {
    $perms = og_role_permissions($roles);
    $role_permissions = [];
    foreach ($perms as $perm) {
      $role_permissions = array_merge($role_permissions, array_keys(array_filter($perm)));
    }
    if (!in_array('add user', $role_permissions) && !user_access('administer group')) {
      drupal_set_message(t('You do not have access to this function. Please configure the needed permission'),
        'error');
    }
    else {
      if (is_numeric($group_id)) {
        og_context($group_type, node_load($group_id));
        $view = views_get_view('springboard_group_bulk_add');
        $view->set_arguments(array($group_id));
        $view->set_display('block_1');
        $view->override_url = $_GET['q'];
        $content = $view->preview();
      }
    }
  }
  return $content;
}