<?php

/**
 * Implements hook_views_data().
 */
function fundraiser_entity_views_data() {
  // This is here to prevent entity api from adding data from the entity properties
}

/**
 * Implements hook_views_data_alter().
 */
function fundraiser_entity_views_data_alter(&$data) {
  // Make the did value fieldable/sortable
  $data['fundraiser_donation']['did'] = array(
    'title' => t('Donation ID'),
    'help' => t('Id of the donation.'),
    'relationship' => array(
      'base' => 'commerce_order',
      'base field' => 'order_id',
      'handler' => 'views_handler_relationship',
      'label' => t('Commerce order'),
    ),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
}
