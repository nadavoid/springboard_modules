<?php

/**
 * @file
 * Definition of views_handler_field_target_actions.
 */

/**
 * A handler to provide group links.
 */
class springboard_group_views_handler_groups_users extends views_handler_field {

  function render($values) {
    return $this->render_groups($values);
  }

  function render_groups($values) {
    if (!empty($values->uid)) {
      $user = user_load($values->uid);
    }

    if (!empty($user)) {
      $groups = array();
      foreach (springboard_group_get_contact_group_gids($user->uid) as $gid) {
        $group_node = node_load($gid);
        if ($group_node->type == 'springboard_group') {
          $groups[] = l($group_node->title, 'node/' . $group_node->nid);
        }
      }
      if (count($groups) > 1) {
        return theme('item_list', array('items' => $groups));
      }
      else {
        return implode(' ', $groups);
      }
    }
    return '';
  }

}


