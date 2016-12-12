<?php

/**
 * Create the fund group entity.
 */
function _fundraiser_fund_group_install_default_entity() {
  entity_save('fundraiser_fund_group',
    entity_create('fundraiser_fund_group_type',
      array(
        'label' => 'Fundraiser Fund Group',
        'type' => 'fundraiser_fund_group_default',
      )
    )
  );

  $fields = array();
  $fields['field_ffg_fd_reference'] = fundraiser_fund_group_fund_reference_field();
  $fields['field_ffg_description'] = fundraiser_fund_group_description_field();
  $fields['field_ffg_image'] = fundraiser_fund_group_image_field();
  $fields['field_ffg_form_id'] = fundraiser_fund_group_form_reference_field();
  $fields['field_ffg_quantity'] = fundraiser_fund_group_quantity_field();
  $fields['field_ffg_other_amt'] = fundraiser_fund_group_other_amount_field();



  foreach ($fields as $key => $values) {
    $base = field_info_field($key);
    if (empty($base)) {
      field_create_field($values['base']);
    }
    field_create_instance($values['instance']);
  }

  fundraiser_fund_group_create_field_groups();

  $fundraiser_fund_group_types = variable_get('fundraiser_fund_group_node_types', array(''));
  $fundraiser_fund_group_types = array_merge($fundraiser_fund_group_types, array('fundraiser_fund_group' => 'fundraiser_fund_group_default'));
  variable_set('fundraiser_fund_group_node_types', $fundraiser_fund_group_types);
}

/**
 * Field definition..
 */
function fundraiser_fund_group_fund_reference_field() {

  $field = array(
    'base' => array(
      'active' => 1,
      'cardinality' => -1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_ffg_fd_reference',
      'foreign keys' => array(
        'commerce_product' => array(
          'columns' => array(
            'target_id' => 'product_id',
          ),
          'table' => 'commerce_product',
        ),
      ),
      'indexes' => array(
        'target_id' => array(
          0 => 'target_id',
        ),
      ),
      'locked' => 0,
      'module' => 'entityreference',
      'settings' => array(
        'handler' => 'base',
        'handler_settings' => array(
          'behaviors' => array(
            'views-select-list' => array(
              'status' => 0,
            ),
          ),
          'sort' => array(
            'type' => 'none',
          ),
          'target_bundles' => array(
            'fundraiser_designation' => 'fundraiser_designation',
          ),
        ),
        'target_type' => 'commerce_product',
      ),
      'translatable' => 0,
      'type' => 'entityreference',
    ),
    'instance' => array(
      'bundle' => 'fundraiser_fund_group_default',
      'default_value' => NULL,
      'default_value_function' => '',
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'module' => 'entityreference',
          'settings' => array(
            'link' => FALSE,
          ),
          'type' => 'entityreference_label',
          'weight' => 2,
        ),
      ),
      'entity_type' => 'fundraiser_fund_group',
      'field_name' => 'field_ffg_fd_reference',
      'label' => 'Funds',
      'required' => 0,
      'settings' => array(
        'behaviors' => array(
          'prepopulate' => array(
            'status' => 0,
          ),
        ),
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'options',
        'settings' => array(),
        'type' => 'options_buttons',
        'weight' => 3,
      ),
    ),
  );

  return $field;
}

/**
 * Field definition..
 */
function fundraiser_fund_group_description_field() {
  $field = array(
    'base' => array(
      'active' => 1,
      'cardinality' => 1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_ffg_description',
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
      'settings' => array(),
      'translatable' => 0,
      'type' => 'text_long',
    ),
    'instance' => array(
      'bundle' => 'fundraiser_fund_group_default',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'module' => 'text',
          'settings' => array(),
          'type' => 'text_default',
          'weight' => 1,
        ),
      ),
      'entity_type' => 'fundraiser_fund_group',
      'field_name' => 'field_ffg_description',
      'label' => 'Description',
      'required' => 0,
      'settings' => array(
        'text_processing' => 1,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'rows' => 5,
        ),
        'type' => 'text_textarea',
        'weight' => 2,
      ),

    ),
  );
  return $field;
}

/**
 * Field definition..
 */
function fundraiser_fund_group_image_field() {
  $field = array(
    'base' => array(
      'active' => 1,
      'cardinality' => 1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_ffg_image',
      'foreign keys' => array(
        'fid' => array(
          'columns' => array(
            'fid' => 'fid',
          ),
          'table' => 'file_managed',
        ),
      ),
      'indexes' => array(
        'fid' => array(
          0 => 'fid',
        ),
      ),
      'locked' => 0,
      'module' => 'image',
      'settings' => array(
        'default_image' => 0,
        'uri_scheme' => 'public',
      ),
      'translatable' => 0,
      'type' => 'image',
    ),
    'instance' => array(
      'bundle' => 'fundraiser_fund_group_default',
      'deleted' => 0,
      'description' => 'Upload a fund group image.',
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'module' => 'image',
          'settings' => array(
            'image_link' => '',
            'image_style' => '',
          ),
          'type' => 'image',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'fundraiser_fund_group',
      'field_name' => 'field_ffg_image',
      'label' => 'Image',
      'required' => 0,
      'settings' => array(
        'alt_field' => 0,
        'default_image' => 0,
        'file_directory' => '',
        'file_extensions' => 'png gif jpg jpeg',
        'max_filesize' => '',
        'max_resolution' => '',
        'min_resolution' => '',
        'title_field' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'image',
        'settings' => array(
          'preview_image_style' => 'thumbnail',
          'progress_indicator' => 'throbber',
        ),
        'type' => 'image_image',
        'weight' => 1,
      ),
    ),
  );
  return $field;
}

/**
 * Field definition..
 */
function fundraiser_fund_group_form_reference_field() {
  $field = array(
    'base' => array(
      'active' => 1,
      'cardinality' => 1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_ffg_form_id',
      'foreign keys' => array(),
      'indexes' => array(
        'target_id' => array(
          0 => 'target_id',
        ),
      ),
      'locked' => 0,
      'module' => 'entityreference',
      'settings' => array(
        'handler' => 'base',
        'handler_settings' => array(
          'behaviors' => array(
            'views-select-list' => array(
              'status' => 0,
            ),
          ),
          'sort' => array(
            'type' => 'none',
          ),
          'target_bundles' => array(),
        ),
        'target_type' => 'node',
      ),
      'translatable' => 0,
      'type' => 'entityreference',
    ),
    'instance' => array(
      'bundle' => 'fundraiser_fund_group_default',
      'default_value' => NULL,
      'default_value_function' => 'entityreference_prepopulate_field_default_value',
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
      ),
      'entity_type' => 'fundraiser_fund_group',
      'field_name' => 'field_ffg_form_id',
      'label' => 'Fund Group ID',
      'required' => 0,
      'settings' => array(
        'behaviors' => array(
          'prepopulate' => array(
            'action' => 'hide',
            'action_on_edit' => 1,
            'fallback' => 'form_error',
            'providers' => array(
              'ffg_node_url' => 1,
              'og_context' => FALSE,
              'url' => 0,
            ),
            'skip_perm' => 0,
            'status' => 1,
          ),
        ),
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'entityreference',
        'settings' => array(
          'match_operator' => 'CONTAINS',
          'path' => '',
          'size' => 60,
        ),
        'type' => 'entityreference_autocomplete',
        'weight' => 4,
      ),
    ),
  );
  return $field;
}

function fundraiser_fund_group_quantity_field() {
  $field = array(
    'base' => array(
      'active' => 1,
      'cardinality' => 2,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_ffg_quantity',
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
          1 => 'Enabled',
        ),
        'allowed_values_function' => '',
      ),
      'translatable' => 0,
      'type' => 'list_text',
      ),
      'instance' => array(
        'bundle' => 'fundraiser_fund_group_default',
        'default_value' => array(
          0 => array(
            'value' => 0,
          ),
        ),
        'deleted' => 0,
        'description' => 'Check this box to enable a quantity field on the group.',
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'module' => 'list',
            'settings' => array(),
            'type' => 'hidden',
            'weight' => 3,
          ),
          'teaser' => array(
            'label' => 'hidden',
            'settings' => array(),
            'type' => 'hidden',
            'weight' => 0,
          ),
        ),
        'entity_type' => 'fundraiser_fund_group',
        'field_name' => 'field_ffg_quantity',
        'label' => 'Quantity field',
        'required' => 0,
        'settings' => array(
          'user_register_form' => FALSE,
        ),
        'widget' => array(
          'active' => 1,
          'module' => 'options',
          'settings' => array(),
          'type' => 'options_buttons',
          'weight' => 31,
        ),
    ),
  );
  return $field;
}

function fundraiser_fund_group_other_amount_field() {
  $field = array(
    'base' => array(
      'active' => 1,
      'cardinality' => 2,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'field_ffg_other_amt',
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
          1 => 'Enabled',
        ),
        'allowed_values_function' => '',
      ),
      'translatable' => 0,
      'type' => 'list_text',
    ),
    'instance' => array(
      'bundle' => 'fundraiser_fund_group_default',
      'default_value' => array(
        0 => array(
          'value' => 0,
        ),
      ),
      'deleted' => 0,
      'description' => 'Check this box to enable an other amount field on the group.',
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'module' => 'list',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 3,
        ),
        'teaser' => array(
          'label' => 'hidden',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'fundraiser_fund_group',
      'field_name' => 'field_ffg_other_amt',
      'label' => 'Other Amount field',
      'required' => 0,
      'settings' => array(
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'options',
        'settings' => array(),
        'type' => 'options_buttons',
        'weight' => 31,
      ),
    ),
  );
  return $field;
}

/**
 * Create the node edit form field groups.
 */
function fundraiser_fund_group_create_field_groups() {

  $export = array();

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_fgf_options|fundraiser_fund_group|fundraiser_fund_group_default|form';
  $field_group->group_name = 'group_fgf_options';
  $field_group->entity_type = 'fundraiser_fund_group';
  $field_group->bundle = 'fundraiser_fund_group_default';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->label = 'Group Options';
  $field_group->weight = '2';
  $field_group->children = array(
    0 => 'field_ffg_description',
    1 => 'field_ffg_image',
    2 => 'field_ffg_quantity',
    3 => 'field_ffg_other_amt',

  );
  $field_group->format_type = 'fieldset';
  $field_group->format_settings = array(
    'label' => 'Group Options',
    'instance_settings' => array(
      'required_fields' => 1,
      'classes' => 'ffg-group-options field-group-fieldset',
      'description' => '',
    ),
    'formatter' => 'collapsible',
  );
  $export['group_fgf_options|fundraiser_fund_group|fundraiser_fund_group_default|form'] = $field_group;

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_fgf_funds|fundraiser_fund_group|fundraiser_fund_group_default|form';
  $field_group->group_name = 'group_fgf_funds';
  $field_group->entity_type = 'fundraiser_fund_group';
  $field_group->bundle = 'fundraiser_fund_group_default';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->label = 'Designation Funds';
  $field_group->weight = '6';
  $field_group->children = array(
    0 => 'field_ffg_fd_reference',
  );
  $field_group->format_type = 'fieldset';
  $field_group->format_settings = array(
    'label' => 'Fund designations',
    'instance_settings' => array(
      'required_fields' => 1,
      'classes' => 'ffg-group-funds field-group-fieldset',
      'description' => '',
    ),
    'formatter' => 'collapsible',
  );
  $export['group_fgf_funds|fundraiser_fund_group|fundraiser_fund_group_default|form'] = $field_group;

  foreach ($export as $field_group) {
    field_group_group_save($field_group);
  }
}


/**
 * Delete springboard message fields.
 */
function _fundraiser_fund_group_delete_default_entity_fields() {
  $fields = array(
    'field_ffg_description',
    'field_ffg_fd_reference',
    'field_ffg_form_id',
    'field_ffg_image',
    'field_ffg_quantity',
  );

  foreach ($fields as $field_name) {
    $field = field_info_instance('node', $field_name, 'fundraiser_fund_group');
    field_delete_instance($field);
  }

  drupal_set_message('Fund group fields deleted.');
  field_group_field_attach_delete_bundle('fundraiser_fund_group', 'fundraiser_fund_group_default');
  $entity_type = fundraiser_fund_group_type_load('fundraiser_fund_group_default');
  if ($entity_type) {
    fundraiser_fund_group_type_delete($entity_type);
  }
}
