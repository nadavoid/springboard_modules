<?php

function _fundraiser_tickets_create_fundraiser_ticket_product() {
  // First check if the product type exists already and if not add
  $product_entities = commerce_product_entity_info();
  if (!isset($product_entities['commerce_product']['bundles']['fundraiser_ticket'])) {
    $product_type = commerce_product_ui_product_type_new();

    $product_type['type'] = 'fundraiser_ticket';
    $product_type['name'] = t('Fundraiser Tickets');
    $product_type['description'] = t('Tickets for a fundraising event.');
    $product_type['is_new'] = TRUE;

    commerce_product_ui_product_type_save($product_type, FALSE);
    commerce_price_create_instance('commerce_price', 'commerce_product', 'fundraiser_ticket', st('Price'), 0, 'calculated_sell_price', array('label' => 'above', 'settings' => array(),'type' => 'hidden','weight' => 2));
  }

  // Now check for fields and if not there add them
  $field_data = _fundraiser_tickets_product_fields();

  foreach ($field_data as $field_name => $field_details){
    $field_id = field_info_field($field_name);
    $instance_id = field_info_instance('commerce_product', $field_name, 'fundraiser_ticket');

    if (empty($field_id)) {
      field_create_field($field_data[$field_name]['field_config']);
    }
    if (empty($instance_id)) {
      field_create_instance($field_data[$field_name]['field_instance']);
    }
  }
}

/**
 * Implementation of hook_commerce_product_type_info()
 */
function _fundraiser_tickets_product_fields() {

  $fields = array();

  $fields['fr_tickets_quantity'] = array(
    'field_config' => array(
      'field_name' => 'fr_tickets_quantity',
      'label' => t('Quantity'),
      'cardinality' => 1,
      'type' => 'number_integer',
      'module' => 'number',
      'active' => 1,
      'locked' => 1,
      'settings' => array(
        'size' => 5,
        'max_length' => 8,
        'text_processing' => 0,
      ),
    ),
    'field_instance' => array(
      'bundle' =>'fundraiser_ticket',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'fr_tickets_quantity',
      'label' => 'Quantity',
      'required' => 1,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 3,
        ),
        'type' => 'text_textfield',
        'weight' => 3,
      ),
    )
  );



  $fields['fr_tickets_quantity_displayed'] = array(
    'field_config' => array(
      'field_name' => 'fr_tickets_quantity_displayed',
      'label' => t('Quantity displayed'),
      'cardinality' => 1,
      'type' => 'number_integer',
      'module' => 'number',
      'active' => 1,
      'locked' => 1,
      'settings' => array(
        'size' => 5,
        'max_length' => 8,
        'text_processing' => 0,
      ),
    ),
    'field_instance' => array(
      'bundle' =>'fundraiser_ticket',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => 'Display this number of tickets remaining unless the actual number of tickets remaining is less than this.',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'fr_tickets_quantity_displayed',
      'label' => 'Quantity displayed',
      'required' => 0,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 3,
        ),
        'type' => 'text_textfield',
        'weight' => 3,
      ),
    )
  );



  $fields['fr_tickets_quantity_sold'] = array(
    'field_config' => array(
      'field_name' => 'fr_tickets_quantity_sold',
      'label' => t('Quantity sold'),
      'cardinality' => 1,
      'type' => 'number_integer',
      'module' => 'number',
      'active' => 1,
      'locked' => 1,
      'settings' => array(
        'size' => 5,
        'max_length' => 8,
        'text_processing' => 0,
      ),
    ),
    'field_instance' => array(
      'bundle' =>'fundraiser_ticket',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'fr_tickets_quantity_sold',
      'label' => 'Quantity sold',
      'required' => 0,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 3,
        ),
        'type' => 'text_textfield',
        'weight' => 3,
      ),
    )
  );



  $fields['fr_tickets_threshold'] = array(
      'field_config' => array(
        'field_name' => 'fr_tickets_threshold',
        'label' => t('Warning threshold'),
        'cardinality' => 1,
        'type' => 'number_integer',
        'module' => 'number',
        'active' => 1,
        'locked' => 1,
        'settings' => array(
          'size' => 5,
          'max_length' => 8,
          'text_processing' => 0,
        ),
      ),
      'field_instance' => array(
        'bundle' =>'fundraiser_ticket',
        'default_value' => NULL,
        'deleted' => 0,
        'description' => '',
        'display' => array(
          'default' => array(
            'label' => 'above',
            'settings' => array(),
            'type' => 'hidden',
            'weight' => 2,
          ),
          'teaser' => array(
            'label' => 'above',
            'settings' => array(),
            'type' => 'hidden',
            'weight' => 0,
          ),
        ),
        'entity_type' => 'commerce_product',
        'field_name' => 'fr_tickets_threshold',
        'label' => 'Warning threshold',
        'required' => 0,
        'settings' => array(
          'text_processing' => 0,
          'user_register_form' => FALSE,
        ),
        'widget' => array(
          'active' => 1,
          'module' => 'text',
          'settings' => array(
            'size' => 3,
          ),
          'type' => 'text_textfield',
          'weight' => 3,
        )
      )
    );

  $fields['fr_tickets_sold_out_message'] = array(
     'field_config' => array(
      'active' => 1,
      'cardinality' => 1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'fr_tickets_sold_out_message',
      'locked' => 0,
      'module' => 'text',
      'settings' => array(
        'max_length' => 255,
      ),
      'translatable' => 0,
      'type' => 'text',
    ),
    'field_instance' => array(
      'bundle' =>'fundraiser_ticket',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'fr_tickets_sold_out_message',
      'label' => 'Sold out message',
      'required' => 0,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 128,
        ),
        'type' => 'text_textfield',
        'weight' => 3,
      ),
    )
  );


  $fields['fr_tickets_description'] = array(
     'field_config' => array(
      'active' => 1,
      'cardinality' => 1,
      'deleted' => 0,
      'entity_types' => array(),
      'field_name' => 'fr_tickets_description',
      'locked' => 0,
      'module' => 'text',
      'settings' => array(
        'max_length' => 255,
      ),
      'translatable' => 0,
      'type' => 'text',
    ),
    'field_instance' => array(
      'bundle' =>'fundraiser_ticket',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'fr_tickets_description',
      'label' => 'Description',
      'required' => 0,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 128,
        ),
        'type' => 'text_textfield',
        'weight' => 2,
      ),
    )
  );

  $fields['fr_tickets_weight'] = array(
    'field_config' => array(
      'field_name' => 'fr_tickets_weight',
      'label' => t('Weight'),
      'cardinality' => 1,
      'type' => 'number_integer',
      'module' => 'number',
      'active' => 1,
      'locked' => 1,
      'settings' => array(
        'size' => 8,
        'max_length' => 10,
        'text_processing' => 0,
      ),
    ),
    'field_instance' => array(
      'bundle' =>'fundraiser_ticket',
      'default_value' => NULL,
      'deleted' => 0,
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'above',
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 0,
        ),
      ),
      'entity_type' => 'commerce_product',
      'field_name' => 'fr_tickets_weight',
      'label' => 'Weight',
      'required' => 0,
      'settings' => array(
        'text_processing' => 0,
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'text',
        'settings' => array(
          'size' => 3,
        ),
        'type' => 'text_textfield',
        'weight' => 3,
      )
    )
  );


  return $fields;
}
