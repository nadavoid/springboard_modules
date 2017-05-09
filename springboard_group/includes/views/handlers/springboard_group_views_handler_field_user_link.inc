<?php

/**
 * @file
 * Definition of springboard_group_views_handler_field_user_link.
 */

/**
 * Field handler to present a link to the user, filtered by group ownership.
 *
 * @ingroup views_field_handlers
 */
class springboard_group_views_handler_field_user_link extends views_handler_field_user_link {

  var $user_groups = array();

  function construct() {
    parent::construct();

    // Put the current user's groups into an array.
    if (user_access('administer group')) {
      $groups = springboard_group_get_all_springboard_groups();
    }
    else {
      $groups = og_get_groups_by_user(NULL, 'node');
    }
    if (!empty($groups)) {
      foreach ($groups as $gid) {
        $this->user_groups[] = $gid;
      }
    }
  }

  // An example of field level access control.
  function access() {
    foreach ($this->user_groups as $gid) {
      if (springboard_group_user_check_group_perms($gid, 'view group contacts')) {
        return TRUE;
      }
    }
    return user_access('access user profiles');
  }

  function render($values) {
    $contact = user_load($values->uid);
    $contact_gids = array();
    if (!empty($contact->og_group_ref_user)) {
      $contact_wrapper = entity_metadata_wrapper('user', $contact);
      $groups = $contact_wrapper->og_group_ref_user->value();
      foreach ($groups as $group) {
        $contact_gids[] = $group->nid;
      }
    }

    $access = FALSE;
    foreach ($this->user_groups as $gid) {
      if (
      (in_array($gid, $contact_gids) && springboard_group_user_check_group_perms($gid, 'view group contacts'))
      || user_access('administer groups')
      || user_access('administer users')
      || user_access('access user profiles')) {
        $access = TRUE;
      }
    }

    if ($access) {
      $value = $this->get_value($values, 'uid');
      return $this->render_link($this->sanitize_value($value), $values);
    }
  }

}
