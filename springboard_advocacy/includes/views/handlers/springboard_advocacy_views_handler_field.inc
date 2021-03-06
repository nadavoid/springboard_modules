<?php

/**
 * @file springboard_advocacy_views_handler_field.inc
 *
 */

class springboard_advocacy_views_handler_field extends views_handler_field
{
  /**
   * Override the parent method
   * We aren't using a DB, so we won't have to lookup aliases
   */
  function query()
  {
    $this->field_alias = $this->real_field;
  }

  /**
   * Make target search results click sortable
   * @param $order
   */
  function click_sort($order)
  {
    if (isset($this->field_alias)) {
      $params = $this->options['group_type'] != 'group' ? array('function' => $this->options['group_type']) : array();
      $this->query->add_orderby(NULL, NULL, $order, $this->field_alias, $params);
    }
  }

  function get_value($values, $field = NULL) {
    $alias = isset($field) ? $this->aliases[$field] : $this->field_alias;

    if($alias == 'committee_memberships' && isset($values->committee_memberships)) {
      foreach($values->committee_memberships as $membership) {
        $result[] = $membership->committee_name;
      };
      return implode("<br />", $result);

    }
    else if (isset($values->{$alias})) {
      return $values->{$alias};
    }
  }

  function render($values) {

    $value = $this->get_value($values);
    $alias = isset($field) ? $this->aliases[$field] : $this->field_alias;

    if($alias == 'committee_memberships') {
      $value = $this->sanitize_value($value);
      return str_replace('&lt;br /&gt;', '<br />', $value);

    }

    return $this->sanitize_value($value);
  }
}