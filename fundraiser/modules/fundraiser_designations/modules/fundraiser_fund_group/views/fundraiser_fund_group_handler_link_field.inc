<?php

/**
 * @file
 * Contains a Views field handler to take care of displaying links to entities
 * as fields.
 */

class fundraiser_fund_group_handler_link_field extends views_handler_field {
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

    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    $fundraiser_fund_group_id = $values->{$this->aliases['fundraiser_fund_group_id']};
    $message = fundraiser_fund_group_load($fundraiser_fund_group_id);
    $wrapper = entity_metadata_wrapper('fundraiser_fund_group', $message);
    $nid = $wrapper->field_ffg_form_id->value()->nid;
    return l($text, 'node/' . $nid . '/designations/fundraiser_fund_group/' . $fundraiser_fund_group_id);
  }
}
