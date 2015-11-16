<?php
/**
 * @file
 * fundraiser_entity.features.field_instance.inc
 */

/**
 * Implements hook_field_default_field_instances().
 */
function fundraiser_entity_field_default_field_instances() {
  $field_instances = array();

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_cc_exp_month'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_cc_exp_month'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'number',
        'settings' => array(
          'decimal_separator' => '.',
          'prefix_suffix' => TRUE,
          'scale' => 0,
          'thousand_separator' => ' ',
        ),
        'type' => 'number_integer',
        'weight' => 9,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_cc_exp_month',
    'label' => 'Credit Card Exp Month',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'max' => '',
      'min' => '',
      'prefix' => '',
      'suffix' => '',
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 0,
      'module' => 'number',
      'settings' => array(),
      'type' => 'number',
      'weight' => 15,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_cc_exp_year'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_cc_exp_year'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'number',
        'settings' => array(
          'decimal_separator' => '.',
          'prefix_suffix' => TRUE,
          'scale' => 0,
          'thousand_separator' => ' ',
        ),
        'type' => 'number_integer',
        'weight' => 8,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_cc_exp_year',
    'label' => 'Credit Card Exp Year',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'max' => '',
      'min' => '',
      'prefix' => '',
      'suffix' => '',
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 0,
      'module' => 'number',
      'settings' => array(),
      'type' => 'number',
      'weight' => 16,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_cc_number'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_cc_number'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 4,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_cc_number',
    'label' => 'Credit Card Number',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 14,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_cc_type'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_cc_type'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'list',
        'settings' => array(),
        'type' => 'list_default',
        'weight' => 6,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_cc_type',
    'label' => 'Credit Card Type',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_select',
      'weight' => 17,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_cid'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_cid'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 23,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_cid',
    'label' => 'Campaign ID',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'rows' => 5,
      ),
      'type' => 'text_textarea',
      'weight' => 11,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_donor_address'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_donor_address'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => array(
      0 => array(
        'element_key' => 'fundraiser_donation|fundraiser_donation|field_fde_donor_address|und|0',
        'thoroughfare' => '',
        'premise' => '',
        'locality' => '',
        'administrative_area' => '',
        'postal_code' => '',
        'country' => 'US',
        'first_name' => '',
        'last_name' => '',
      ),
    ),
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'administrator' => array(
        'label' => 'above',
        'module' => 'addressfield',
        'settings' => array(
          'format_handlers' => array(
            'address' => 'address',
            'name-full' => 'name-full',
          ),
          'use_widget_handlers' => 0,
        ),
        'type' => 'addressfield_default',
        'weight' => 1,
      ),
      'default' => array(
        'label' => 'above',
        'module' => 'addressfield',
        'settings' => array(
          'format_handlers' => array(
            'address' => 'address',
            'name-full' => 'name-full',
          ),
          'use_widget_handlers' => 0,
        ),
        'type' => 'addressfield_default',
        'weight' => 1,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_donor_address',
    'label' => 'Donor Address',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'addressfield',
      'settings' => array(
        'available_countries' => array(),
        'format_handlers' => array(
          'address' => 'address',
          'name-full' => 'name-full',
          'address-hide-country' => 0,
          'organisation' => 0,
          'name-oneline' => 0,
        ),
      ),
      'type' => 'addressfield_standard',
      'weight' => 9,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_eft_account_number'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_eft_account_number'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 3,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_eft_account_number',
    'label' => 'Account Number',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 13,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_eft_routing_number'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_eft_routing_number'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 2,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_eft_routing_number',
    'label' => 'Routing Number',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 10,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_email'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_email'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 15,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_email',
    'label' => 'Email',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 20,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_initial_referrer'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_initial_referrer'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 19,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_initial_referrer',
    'label' => 'Initial referrer',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 23,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_ms'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_ms'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 22,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_ms',
    'label' => 'Market Source',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'rows' => 5,
      ),
      'type' => 'text_textarea',
      'weight' => 12,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_payment_method'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_payment_method'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 10,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_payment_method',
    'label' => 'Payment Method',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 18,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_referrer'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_referrer'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 18,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_referrer',
    'fundraiser_entity' => array(
      'webform_map' => 1,
    ),
    'label' => 'Referrrer',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'webform_map' => 1,
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'size' => 60,
      ),
      'type' => 'text_textfield',
      'weight' => 22,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_search_engine'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_search_engine'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 20,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_search_engine',
    'label' => 'Search engine',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 24,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_search_string'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_search_string'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 21,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_search_string',
    'label' => 'Search string',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
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
      'weight' => 25,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_sustainer'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_sustainer'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'list',
        'settings' => array(),
        'type' => 'list_default',
        'weight' => 14,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_sustainer',
    'label' => 'Sustainer',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_buttons',
      'weight' => 19,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_transaction_date'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_transaction_date'] = array(
    'bundle' => 'fundraiser_donation',
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'date',
        'settings' => array(
          'format_type' => 'long',
          'fromto' => 'both',
          'multiple_from' => '',
          'multiple_number' => '',
          'multiple_to' => '',
        ),
        'type' => 'date_default',
        'weight' => 27,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_transaction_date',
    'label' => 'Transaction Date',
    'required' => 0,
    'settings' => array(
      'default_value' => 'now',
      'default_value2' => 'same',
      'default_value_code' => '',
      'default_value_code2' => '',
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'date',
      'settings' => array(
        'increment' => 15,
        'input_format' => 'm/d/Y - H:i:s',
        'input_format_custom' => '',
        'label_position' => 'above',
        'text_parts' => array(),
        'year_range' => '-3:+3',
      ),
      'type' => 'date_text',
      'weight' => 26,
    ),
  );

  // Exported field_instance: 'fundraiser_donation-fundraiser_donation-field_fde_user_agent'
  $field_instances['fundraiser_donation-fundraiser_donation-field_fde_user_agent'] = array(
    'bundle' => 'fundraiser_donation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 11,
      ),
    ),
    'entity_type' => 'fundraiser_donation',
    'field_name' => 'field_fde_user_agent',
    'label' => 'User Agent',
    'required' => 0,
    'settings' => array(
      'fundraiser_entity' => array(
        'webform_map' => 1,
      ),
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'rows' => 5,
      ),
      'type' => 'text_textarea',
      'weight' => 21,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Account Number');
  t('Campaign ID');
  t('Credit Card Exp Month');
  t('Credit Card Exp Year');
  t('Credit Card Number');
  t('Credit Card Type');
  t('Donor Address');
  t('Email');
  t('Initial referrer');
  t('Market Source');
  t('Payment Method');
  t('Referrrer');
  t('Routing Number');
  t('Search engine');
  t('Search string');
  t('Sustainer');
  t('Transaction Date');
  t('User Agent');

  return $field_instances;
}
