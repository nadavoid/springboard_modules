<?php

/**
 * @file
 * Builds placeholder replacement tokens for line item related data.
 */

/**
 * Implements hook_token_info_alter().
 */
function fundraiser_designations_token_info_alter(&$data) {
  if (!empty($data['tokens']['donation'])) {
    $data['tokens']['donation']['designations-line-items'] = array(
      'name' => t('Designations'),
      'description' => t('Designations selected.'),
    );
  }
}

/**
 * Implements hook_tokens_alter().
 */
function fundraiser_designations_tokens_alter(&$replacements, $context) {

  if ($context['type'] == 'donation' && !empty($context['data']['donation'])) {
    $donation = $context['data']['donation'];
  }

  if (!empty($donation) && !empty($donation->node) && fundraiser_designations_is_designation_type($donation->node->type)) {
    $order = !empty($donation->order) ? $donation->order : commerce_order_load($donation->did);
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

    $designations = [];
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      $item = entity_load_single('commerce_line_item', $line_item_wrapper->line_item_id->value());
      $designations[$delta]['name'] = $line_item_wrapper->line_item_label->value();
      $designations[$delta]['quantity'] = $line_item_wrapper->quantity->value();
      $designations[$delta]['amount'] = $line_item_wrapper->commerce_unit_price->value();
      $designations[$delta]['pid'] = $line_item_wrapper->commerce_product->product_id->value();
      $designations[$delta]['type'] = $item->data['type'];
    }

    $yadda = theme('fundraiser_designations_line_item_token', array('line_items' => $designations, 'donation' => $donation));
    $data['designations-line-items'] = $yadda;

    foreach ($context['tokens'] as $key => $token) {
      if (isset($data[$key])) {
        $replacements[$token] = $data[$key];
      }
    }
  }
}

/**
 * Theme the line item token output.
 */
function theme_fundraiser_designations_line_item_token($vars) {
  $list = [];
  $vars['donation']->node->fd_addon['pid'];
  $symbol = $vars['donation']->donation['currency']['symbol'];
  foreach ($vars['line_items'] as $delta => $item) {
    $name = $item['name'];
    if ($item['pid'] == $vars['donation']->node->fd_addon['pid'] && $item['type'] == 'addon') {
      $name = t('One time add-on donation - !name', array('!name' => $name));
    }
    $amount = $symbol . commerce_currency_amount_to_decimal($item['quantity'] * $item['amount']['amount'], $item['amount']['currency_code']);
    $li = t('!name: quantity: !quantity, total: @amount.',
      array(
        '!name' => $name,
        '!quantity' => (int) $item['quantity'],
        '@amount' => $amount,
      )
    );
    $list[] = check_markup($li);
  }
  return theme('item_list', array('items' => $list, 'title' => t('Designated Funds')));
}