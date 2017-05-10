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

  function access() {
    return TRUE;
  }

  function render($values) {
    if (springboard_group_user_view_access($values->uid)) {
      $value = $this->get_value($values, 'uid');
      return $this->render_link($this->sanitize_value($value), $values);
    }
  }
}
