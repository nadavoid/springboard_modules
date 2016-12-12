<?php
/**
 * @file
 * Handler for the Used By field.
 */

/**
 * Field handler to present the card expiration date.
 */
class fundraiser_commerce_cardonfile_handler_field_used_by extends views_handler_field_entity {

  function pre_render(&$rows) {
    foreach ($rows as &$values) {
      if (!is_object($values) || empty($values->card_id)) {
        return;
      }

      $used_by = &drupal_static(__METHOD__);

      $card_id = $values->card_id;
      if (empty($used_by[$card_id])) {
        $used_by[$card_id] = module_invoke_all('fundraiser_commerce_cardonfile_used_by', $card_id);
      }

      $values->used_by = $used_by[$card_id];
    }
  }

  function render($values) {
    if (!is_object($values) || !isset($values->used_by)) {
      return '';
    }

    $used_by = $values->used_by;

    $output = '';
    if (count($used_by)) {
      foreach ($used_by as $u) {
        if (!empty($this->options['element_type'])) {
          $element = array(
            'element' => array(
              '#tag' => $this->options['element_type'],
              '#value' => $u,
            ),
          );
          if (!empty($this->options['element_class'])) {
            $element['element']['#attributes']['class'] = $this->option['element_class'];
          }
          $output .= theme('html_tag', $element);
        }
        else {
          $output .= $u . ' ';
        }
      }
    }

    return $output;
  }
}
