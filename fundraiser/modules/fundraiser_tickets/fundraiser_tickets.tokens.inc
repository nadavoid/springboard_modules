<?php

/**
 * @file
 * Builds placeholder replacement tokens for line item related data.
 */

/**
 * Implements hook_token_info_alter().
 */
function fundraiser_tickets_token_info_alter(&$data) {
  if (!empty($data['tokens']['donation'])) {
    // Add some info from the ticket
    $data['tokens']['donation']['tickets-total-amount'] = array(
      'name' => t('Ticket Total Amount'),
      'description' => t('Ticket purchase amount plus the add-on donation amount.'),
    );
    $data['tokens']['donation']['tickets-quantity-all'] = array(
      'name' => t('Ticket Quantity'),
      'description' => t('Quantity of all tickets purchased.'),
    );
    $data['tokens']['donation']['tickets-quantity-types'] = array(
      'name' => t('Ticket Quantity (type)'),
      'description' => t('Quantity of all the different types of tickets purchased.'),
    );
  }
}

/**
 * Implements hook_tokens_alter
 */
function fundraiser_tickets_tokens_alter(&$replacements, $context) {
  // On a webform confirmation page load the donation from the sid
  if ($context['type'] == 'donation' && !empty($context['data']['donation'])) {
    $donation = $context['data']['donation'];
  }

  // If we have a donation and this is a ticket donation form add ticket related tokens
  if (!empty($donation) && !empty($donation->node) && fundraiser_tickets_is_ticket_type($donation->node->type)) {
    // Load the order
    $order = !empty($donation->order) ? $donation->order : commerce_order_load($donation->did);
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $currency = commerce_currency_load($donation->currency);   
    $total_count = 0;
    $type_count = 0;
    // Get totals for the line items
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      $total_count = $total_count + $line_item_wrapper->quantity->value();
      $type_count++;
    }
    // Add additional tokens for specific ticket information
    $data['tickets-quantity-all'] = $total_count;
    $data['tickets-quantity-types'] = $type_count;
    $data['tickets-total-amount'] =  number_format($donation->amount, $currency['decimals'], $currency['decimal_separator'], '');
   
    // Replace the given tokens.
    foreach ($context['tokens'] as $key => $token) {
      if (isset($data[$key])) {
        $replacements[$token] = $data[$key];
      }
    }
  }

  // For the ticket line items reformat the quantity
  if ($context['type'] == 'commerce-line-item'
   && !empty($context['data']['commerce-line-item'])
   && $context['data']['commerce-line-item']->type == 'ticket') {
    $quantity_token = $context['tokens']['quantity'];
    $replacements[$quantity_token] = number_format($replacements[$quantity_token]);
  }
}
