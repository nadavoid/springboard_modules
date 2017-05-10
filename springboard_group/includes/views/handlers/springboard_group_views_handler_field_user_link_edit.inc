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

    $account = new stdClass();
    $account->uid = $data;

    if ($data && springboard_group_user_edit_access($account)) {
      $this->options['alter']['make_link'] = TRUE;

      $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');

      $this->options['alter']['path'] = "user/$data/edit";
      $this->options['alter']['query'] = drupal_get_destination();

      return $text;
    }
  }
}
