<?php

/**
 * @file
 * Fields for fundraiser designations.
 */

/**
 * Creates the extra product fields.
 */
function _fundraiser_designations_create_fundraiser_designation_product() {
  // First check if the product type exists already and if not add.
  $product_entities = commerce_product_entity_info();
  if (!isset($product_entities['commerce_product']['bundles']['fundraiser_designation'])) {
    $product_type = commerce_product_ui_product_type_new();

    $product_type['type'] = 'fundraiser_designation';
    $product_type['name'] = t('Fundraiser Designations');
    $product_type['description'] = t('Designations for a fundraiser donation.');
    $product_type['is_new'] = TRUE;

    commerce_product_ui_product_type_save($product_type, FALSE);
    commerce_price_create_instance('commerce_price', 'commerce_product', 'fundraiser_designation', st('Price'), 0, 'calculated_sell_price', array(
      'label' => 'above',
      'settings' => array(),
      'type' => 'hidden',
      'weight' => 2,
    ));
  }
}