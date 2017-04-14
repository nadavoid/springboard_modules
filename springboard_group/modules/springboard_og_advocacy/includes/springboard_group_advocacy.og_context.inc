<?php

/**
 * Implements hook_springboard_group_context_menu_path().
 *
 * Define the context paths for hook_og_context_negotiation_info().
 *
 * Also called from springboard_group_advocacy_preprocess_page().
 */
function springboard_group_advocacy_springboard_group_context_menu_paths() {

  $paths = &drupal_static(__FUNCTION__);
  if (!isset($paths)) {
    $groups_enabled_types = springboard_group_get_grouped_types();
    $paths = array();
    if (isset($groups_enabled_types['node']['sba_message_action']) || isset($groups_enabled_types['node']['sba_social_action'])) {
      array_push($paths, 'admin/springboard/advocacy/targets');
      array_push($paths, 'admin/springboard/advocacy/actions');
      array_push($paths, 'admin/springboard/advocacy/targets/custom-targets');
      array_push($paths, 'admin/springboard/advocacy/targets/custom-groups');
      array_push($paths, 'admin/springboard/advocacy/targets/custom-targets/add');
      array_push($paths, 'admin/springboard/advocacy/targets/custom-groups/add');
    }
    if (isset($groups_enabled_types['node']['sba_message_action'])) {
      array_push($paths, 'admin/springboard/advocacy/message-actions/all');
    }
    if (isset($groups_enabled_types['node']['sba_social_action'])) {
      array_push($paths, 'admin/springboard/advocacy/social-actions/all');
    }
  }

  return $paths;
}
