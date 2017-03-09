<?php
/**
 * @file
 * Handler for the Used For field.
 */

/**
 * Field handler to present the card expiration date.
 */
class fundraiser_commerce_cardonfile_handler_field_used_for extends views_handler_field_entity {

  function pre_render(&$rows) {
    foreach ($rows as &$values) {
      if (!is_object($values) || empty($values->card_id)) {
        return;
      }

      $used_for = &drupal_static(__METHOD__);

      $card_id = $values->card_id;
      if (empty($used_for[$card_id])) {
        $used_for[$card_id] = module_invoke_all('fundraiser_commerce_cardonfile_used_for', $card_id);
      }

      $values->used_for = $used_for[$card_id];
    }
  }

  function render($values) {
    if (!is_object($values) || !isset($values->used_for)) {
      return '';
    }

    $used_for = $values->used_for;

    $output = '';
    if (count($used_for)) {
      foreach ($used_for as $u) {
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
