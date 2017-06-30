<?php

/**
 * @file
 * Definition of views_handler_field_target_actions.
 */

/**
 * A handler to provide group links.
 */
class springboard_group_views_handler_groups extends views_handler_field {

  function render($values) {
    return $this->render_groups($values);
  }

  function render_groups($values) {
    if (!empty($values->nid)) {
      $node = node_load($values->nid);
      $groups = array();
      foreach (og_get_entity_groups('node', $node) as $entity_type => $og_memberships) {
        foreach ($og_memberships as $membership_id => $entity_id) {
          if ($entity_type == 'node') {
            $group_node = node_load($entity_id);
            if ($group_node->type == 'springboard_group') {
              $groups[] = l($group_node->title, 'node/' . $group_node->nid);
            }
          }
        }
      }
      if (count($groups) > 1) {
        asort($groups);
        return theme('item_list', array('items' => $groups));
      }
      else {
        return implode(' ', $groups);
      }
    }
    return '';
  }

}


