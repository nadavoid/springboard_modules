<?php

/**
 * @file
 * Definition of springboard_group_views_handler_field_user_link_edit.
 */

/**
 * Field handler to present a link to user edit.
 *
 * @ingroup views_field_handlers
 */
class springboard_group_views_handler_field_user_link_edit extends springboard_group_views_handler_field_user_link {
  function render_link($data, $values) {

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
        (in_array($gid, $contact_gids) && springboard_group_user_check_group_perms($gid, 'manage group contacts'))
        || user_access('administer groups')
        || user_access('administer users')) {
        $access = TRUE;
      }
    }

    $account = new stdClass();
    $account->uid = $data;

    if ($data && (user_edit_access($account) || $access)) {
      $this->options['alter']['make_link'] = TRUE;

      $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');

      $this->options['alter']['path'] = "user/$data/edit";
      $this->options['alter']['query'] = drupal_get_destination();

      return $text;
    }
  }
}
