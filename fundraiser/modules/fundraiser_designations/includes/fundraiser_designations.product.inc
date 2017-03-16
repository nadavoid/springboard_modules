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
    _fundraiser_designations_install_product_fields($product_type['type']);

    commerce_price_create_instance('commerce_price', 'commerce_product', 'fundraiser_designation', st('Price'), 0, 'calculated_sell_price', array(
      'label' => 'above',
      'settings' => array(),
      'type' => 'hidden',
      'weight' => 2,
    ));
  }
}

/**
 * Add an Internal Name field to the Fundraiser Designation prodcut type.
 *
 * @param string $product_type
 *   Type is fundraiser_designation.
 */
function _fundraiser_designations_install_product_fields($product_type) {
  module_load_include('inc', 'field', 'field.crud');
  module_load_include('inc', 'field', 'field.info');

  if (function_exists('_fundraiser_designations_install_product_field_definitions')) {
    foreach (_fundraiser_designations_install_product_field_definitions($product_type) as $field_name => $field) {
      $exists = field_info_field('field_' . $field_name);
      if (empty($exists)) {
        field_create_field($field['field_config']);
      }
      // Before adding the field, check that it doesn't exist on the node type.
      if (!field_info_instance('node', 'field_' . $field_name, $product_type)) {
        field_create_instance($field['field_instance']);
      }
    }
  }
}

/**
 * Product field definitions.
 *
 * @return arrayd
 *   Field definitions.
 */
function _fundraiser_designations_install_product_field_definitions($product_type) {

  $fields['internal_title'] = array(
    'field_config' => array(
      'active' => 1,
      'cardinality' => 1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_internal_title',
      'foreign keys' => array(
        'format' => array(
          'columns' => array(
            'format' => 'format',
          ),
          'table' => 'filter_format',
        ),
      ),
      'indexes' => array(
        'format' => array(
          0 => 'format',
        ),
      ),
      'locked' => 0,
      'module' => 'text',
      'settings' => array(
        'max_length' => 255,
      ),
      'translatable' => 0,
      'type' => 'text',
    ),
    'field_instance' => array(
      'bundle' => 'fundraiser_designation',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'module' => 'text',
          'settings' => array(),
          'type' => 'text_default',
          'weight' => 5,
        ),
        'line_item' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
        'node_teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'field_internal_title',
      'label' => 'Internal Title',
      'required' => 1,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 60,
        ),
        'type' => 'text_textfield',
        'weight' => 2,
      ),
    ),
  );

  return $fields;
}
