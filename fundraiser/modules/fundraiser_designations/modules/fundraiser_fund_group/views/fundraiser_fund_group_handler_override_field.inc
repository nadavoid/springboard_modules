<?php

/**
 * @file
 * Contains a Views field handler to take care of displaying links to entities
 * as fields.
 */

class fundraiser_fund_group_handler_override_field extends views_handler_field {
  function construct() {
    parent::construct();

    $this->additional_fields['fundraiser_fund_group_id'] = 'fundraiser_fund_group_id';
    $this->additional_fields['type'] = 'type';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    $fundraiser_fund_group_id = $values->{$this->aliases['fundraiser_fund_group_id']};
    $message = fundraiser_fund_group_load($fundraiser_fund_group_id);
    if ($message->fd_overrides['status'] == 1) {
      return t('amounts overridden');
    }
    else {
      return '';
    }
  }
}
