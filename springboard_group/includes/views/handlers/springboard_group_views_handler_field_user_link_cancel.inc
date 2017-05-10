<?php

/**
 * @file
 * Definition of springboard_group_views_handler_field_user_link_cancel.
 */

/**
 * Field handler to present a link to user cancel,filtered by group.
 *
 * @ingroup views_field_handlers
 */
class springboard_group_views_handler_field_user_link_cancel extends springboard_group_views_handler_field_user_link {

  function render_link($data, $values) {

    $uid = $values->{$this->aliases['uid']};

    // Build a pseudo account object to be able to check the access.
    $account = new stdClass();
    $account->uid = $uid;

    if ($uid && springboard_group_user_cancel_access($account)) {
      $this->options['alter']['make_link'] = TRUE;

      $text = !empty($this->options['text']) ? $this->options['text'] : t('Cancel account');

      $this->options['alter']['path'] = "user/$uid/cancel";
      $this->options['alter']['query'] = drupal_get_destination();

      return $text;
    }
  }
}
