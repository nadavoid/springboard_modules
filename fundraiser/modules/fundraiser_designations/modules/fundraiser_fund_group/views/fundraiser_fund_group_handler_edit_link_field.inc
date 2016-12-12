<?php

/**
 * @file
 * Contains a Views field handler to take care of displaying edit links
 * as fields
 */


class fundraiser_fund_group_handler_edit_link_field extends fundraiser_fund_group_handler_link_field {
  function construct() {
    parent::construct();
    $this->additional_fields['type'] = 'type';
  }

  function render($values) {
    $type = $values->{$this->aliases['type']};
    //Creating a dummy model to check access against
    $dummy_model = (object) array('type' => $type);
    if (!fundraiser_fund_group_access('edit', $dummy_model)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    $fundraiser_fund_group_id = $values->{$this->aliases['fundraiser_fund_group_id']};
    $message = fundraiser_fund_group_load($fundraiser_fund_group_id);
    $wrapper = entity_metadata_wrapper('fundraiser_fund_group', $message);
    $nid = $wrapper->field_ffg_form_id->value()->nid;
    return l($text, 'node/' . $nid  .'/designations/fundraiser_fund_group/' . $fundraiser_fund_group_id . '/edit');
  }
}
