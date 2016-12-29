<?php

/**
 * @file
 * Install helper functions for Fundraiser Designations taxonomy.
 */

/**
 * Install Fundraiser Designations categories vocabulary.
 *
 */
function _fundraiser_designations_install_vocab_fd_designation_categories($product_type) {
  $already_exists = FALSE;
  foreach (taxonomy_get_vocabularies() as $vocab) {
    if ($vocab->machine_name == 'fd_designation_categories') {
      $already_exists = TRUE;
      break;
    }
  }
  if (!$already_exists) {
    $status = taxonomy_vocabulary_save((object) array(
      'name' => t("Fundraiser Designation Categories"),
      'machine_name' => 'fd_designation_categories',
      'description' => t("Categories for Fund Designations."),
    ));
    if ($status == SAVED_NEW) {
      $vocab = taxonomy_vocabulary_machine_name_load('fd_designation_categories');
      // Populate with Springboard standard terms.
      $weight = 0;
      foreach (_fundraiser_designations_default_categories() as $parent => $children) {
        taxonomy_term_save((object) array(
          'vid' => $vocab->vid,
          'name' => $parent,
          'weight' => $weight++,
          'vocabulary_machine_name' => $vocab->machine_name,
        ));
        $parent_terms = taxonomy_term_load_multiple(array(), array('name' => $parent, 'vid' => $vocab->vid));
        if (!empty($parent_terms)) {
          $parent_term = reset($parent_terms);
          foreach ($children as $child) {
            taxonomy_term_save((object) array(
              'vid' => $vocab->vid,
              'name' => $child,
              'weight' => $weight++,
              'parent' => array($parent_term->tid),
              'vocabulary_machine_name' => $vocab->machine_name,
            ));
          }
        }
      }
    }
    _fundraiser_designations_install_taxonomy_fields($product_type);
    _fundraiser_designations_install_lock_fields();
  }
}

/**
 * Define designation categories default terms.
 *
 * @return array
 *   Default terms.
 */
function _fundraiser_designations_default_categories() {
  return array(
    "General" => array(),
  );
}

/**
 * Install the fundraiser designation categories fields on a product type.
 *
 * @param string $product_type
 *   Type is fundraiser_designation.
 */
function _fundraiser_designations_install_taxonomy_fields($product_type) {
  module_load_include('inc', 'field', 'field.crud');
  module_load_include('inc', 'field', 'field.info');

  if (function_exists('_fundraiser_designations_install_taxonomy_field_definitions')) {
    foreach (_fundraiser_designations_install_taxonomy_field_definitions($product_type) as $field_name => $field) {
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
 * Taxonomy field definitions.
 *
 * @return array
 *   Field definitions.
 */
function _fundraiser_designations_install_taxonomy_field_definitions($product_type) {

  $fields['fd_designation_categories'] = array(
    'field_config' => array(
      'field_name' => 'field_fd_designation_categories',
      'type' => 'taxonomy_term_reference',
      'translatable' => '0',
      'cardinality' => '-1',
      'foreign keys' => array(
        'tid' => array(
          'columns' => array(
            'tid' => 'tid',
          ),
          'table' => 'taxonomy_term_data',
        ),
      ),
      'indexes' => array(
        'tid' => array(
          0 => 'tid',
        ),
      ),
      'locked' => '1',
      'module' => 'taxonomy',
      'settings' => array(
        'allowed_values' => array(
          0 => array(
            'vocabulary' => 'fd_designation_categories',
            'parent' => '0',
          ),
        ),
      ),
    ),
    'field_instance' => array(
      'field_name' => 'field_fd_designation_categories',
      'entity_type' => 'commerce_product',
      'bundle' => 'fundraiser_designation',
      'label' => t("Fund Categories"),
      'default_value' => NULL,
      'required' => 1,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => '4',
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'options',
        'settings' => array(),
        'type' => 'options_buttons',
        'weight' => 5,
      ),
    ),
  );

  return $fields;
}

/**
 * Add a checkbox to the designations category.
 */
function fundraiser_designations_taxonomy_category_field_bases() {
  $field_bases = array();

  $field_bases['field_fd_lock_donation_amount'] = array(
    'active' => 1,
    'cardinality' => 2,
    'deleted' => 0,
    'entity_types' => array(),
    'field_name' => 'field_fd_lock_donation_amount',
    'foreign keys' => array(),
    'indexes' => array(
      'value' => array(
        0 => 'value',
      ),
    ),
    'locked' => 0,
    'module' => 'list',
    'settings' => array(
      'allowed_values' => array(
        1 => 'Lock recurring donation amounts of products with this term.',
      ),
      'allowed_values_function' => '',
    ),
    'translatable' => 0,
    'type' => 'list_text',
  );

  return $field_bases;
}

/**
 * Add a checkbox to the designations category.
 */
function fundraiser_designations_taxonomy_category_field_instances() {
  $field_instances = array();

  $field_instances['field_fd_lock_donation_amount'] = array(
    'bundle' => 'fd_designation_categories',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'Don\'t allow donors to update the amount of their recurring donations to funds in this category.',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'list',
        'settings' => array(),
        'type' => 'list_default',
        'weight' => 1,
      ),
    ),
    'entity_type' => 'taxonomy_term',
    'field_name' => 'field_fd_lock_donation_amount',
    'label' => 'Lock Recurring Donation Amount',
    'required' => 0,
    'settings' => array(
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_buttons',
      'weight' => 1,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Don\'t allow donors to update the amount of their recurring donations to funds in this category.');
  t('Lock Recurring Donation Amount');

  return $field_instances;
}

/**
 * Add a checkbox to the designations category.
 */
function _fundraiser_designations_install_lock_fields() {
  $already_exists = FALSE;
  foreach (taxonomy_get_vocabularies() as $vocab) {
    if ($vocab->machine_name == 'fd_designation_categories') {
      $already_exists = TRUE;
      break;
    }
  }

  if ($already_exists) {
    $fields = fundraiser_designations_taxonomy_category_field_bases();
    $instances = fundraiser_designations_taxonomy_category_field_instances();
    foreach ($fields as $key => $values) {
      $instance = $instances[$key];
      $base = field_info_field($key);
      $term = field_info_instance('taxonomy_term', $key, 'fd_designation_categories');
      if (empty($base)) {
        field_create_field($fields[$key]);
      }
      if (empty($term)) {
        field_create_instance($instance);
      }
    }
  }
}