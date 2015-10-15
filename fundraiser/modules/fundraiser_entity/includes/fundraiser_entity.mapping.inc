<?php

/**
 * Menu callback form, provides a user interface to set up donation entity fields to data source mappings.
 */
function fundraiser_entity_default_mapping_form($form, &$form_state) {
  // Warn an admin if the webform tokens module is not enabled.
  if (!module_exists('webform_tokens')) {
    drupal_set_message('The Webform Tokens module is not enabled. Tokens for webform values will not be replaced.', 'warning');
  }

  $form['info'] = array(
    '#markup' => t('This form allows you to map values to the Donation Entity fields.'),
    '#weight' => -100,
  );

  // Select when the values should be mapped to the entity.
  $form['fundraiser_entity_map_point'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select when the donation entity should be created and the values mapped to the entity.'),
    '#options' => array(
      'create' => t('Donation create'),
      'update' => t('Donation update'),
      'success' => t('On a successful donation'),
      'sustainer_series_cron' => t('Sustainer Series on Cron'),
      'decline' => t('Donation decline'),
    ),
    '#default_value' => variable_get('fundraiser_entity_map_point', array()),
  );

  // Container for component options.
  $form['webform_components'] = array(
    '#type' => 'fieldset',
    '#title' => t('Webform Component Options.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // Select which fields should NOT automatically create fields.
  $form['webform_components']['fundraiser_entity_webform_autocreate_ignore'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select the webform fields that should not automatically create fields on the donation entity.'),
    '#description' => t("Select fields to prevent the system from creating a field on the donation entity when this component is added to a webform."),
    '#options' => _fundraiser_entity_load_all_webform_components(),
    '#default_value' => variable_get('fundraiser_entity_webform_autocreate_ignore', array()),
  );

  $header = array(
    t('Donation Field'),
    t('Data Source'),
    t('Optional token or text'),
  );

  // Table element.
  $form['mapping'] = array(
    '#prefix' => '<div id="fields">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => array(),
  );

  // Load the default mapping.
  $default_map = variable_get('fundraiser_entity_default_map', array());

  $donation_fields = _fundraiser_entity_get_entity_fields();

  // Protect fields that are set in code.
  $protect = array(
    'did',
    'nid',
    'sid',
    'uid',
    'status',
    'created',
    'changed',
    'amount',
    'currency',
    'txn_id',
    'gateway',
    'form_url'
  );

  // Create an array of fields not to show
  $remove = array(
    'url',
  );

  // Check the field settings to see if it should be shown on the mapping form
  foreach ($donation_fields as $name => $label) {
    // Don't bother with the protected fields
    if (in_array($name, $protect)) {
      continue;
    }

    // Get the field name
    $field_name = $name;
    // Check for the subkey value
    if (strpos($field_name, ':') !== FALSE) {
      list($field_name, $sub_key) = explode(':', $field_name);
    }

    // Ensure this field instance can be mapped
    $instance_info = field_info_instance('fundraiser_donation', $field_name, 'fundraiser_donation');
    if (empty($instance_info['settings']['fundraiser_entity']['webform_map'])) {
      $remove[] = $name;
    }
  }

  // Track all the fields in the form.
  $form['#donation_fields'] = $donation_fields;

  // Create an array of options from the related entities.
  $entity_types = array(
    'node' => 'Node',
    'user' => 'User',
    'commerce_order' => 'Order',
    'commerce_payment_transaction' => 'Transaction',
    'commerce_customer_profile' => 'Customer',
  );

  $options[] = '--select--';
  // Add the option for entering a fixed text or token value.
  $options['other'] = 'Token or text';

  foreach ($entity_types as $entity_type => $opt_name) {
    $entity_fields = _fundraiser_entity_get_entity_fields($entity_type, NULL);
    foreach ($entity_fields as $name => $label) {
      $options[$opt_name][ $entity_type . '--' . $name ] = $label;
    }
    asort($options[$opt_name]);
  }

  // Create an array of options from the donation entity fields.
  foreach ($donation_fields as $name => $label) {
    if (in_array($name, $remove)) {
      continue;
    }
    elseif (in_array($name, $protect)) {
      $form['mapping']['#rows'][] = array(
        array('data' => $label),
        array('data' => 'This value is set in code.'),
        '',
      );
    }
    else {
      $default_source = isset($default_map[$name]) ? $default_map[$name]['source'] : '';
      $default_other =  isset($default_map[$name]) ? $default_map[$name]['other'] : '';

      // Create a select field for this entity field.
      $field_id = drupal_clean_css_identifier($name);
      $source = array(
        '#id' => 'donation-field-' . $field_id . '-source',
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => $default_source,
      );

      $other = array(
        '#id' => 'donation-field-' . $field_id . '-other',
        '#type' => 'textfield',
        '#default_value' => $default_other,
        '#states' => array(
          'visible' => array(
            '[id="donation-field-' . $field_id . '-source"]' => array('value' => 'other'),
          ),
        )
      );

      // Add the field to the form.
      $form['mapping'][$name] = array(
        'source' => &$source,
        'other' => &$other,
      );

      // Add the rows to the table.
      $form['mapping']['#rows'][] = array(
        array('data' => $label),
        array('data' => &$source),
        array('data' => &$other),
      );
 
      unset($source, $other);
    }
  }

  $token_set = array(
    'donation',
    'node',
    'user',
    'commerce-order',
    'commerce-payment-transaction',
    'webform-tokens',
    'commerce-customer-profile'
  );

  $form['tokens'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available tokens'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 9,
  );

  // Then add any other token set as needed.
  $form['tokens']['tokens'] = array(
    '#type' => 'item',
    '#title' => t('Drupal tokens'),
    '#description' => theme('token_tree', array('token_types' => $token_set, 'recursion_limit' => 2, 'click_insert' => FALSE)),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  $form['reset'] = array(
    '#type' => 'submit',
    '#value' => 'Reset Map',
    '#submit' => array('fundraiser_entity_default_mapping_form_reset'),
  );

  return $form;

}

/**
 * Validate handler for mapping form.
 */
function fundraiser_entity_default_mapping_form_validate($form, $form_state) {
  foreach ($form_state['values']['mapping'] as $name => $values) {
    if (
      !empty($form['#donation_fields'][$name])
      && !empty($values['source'])
      && $values['source'] == 'other'
      && empty($values['other'])
    ) {
      $field_name = $form['#donation_fields'][$name];
      form_set_error('mapping[' . $name . '][other]', t("You did not enter a token or text value for the @field", array('@field' => $field_name)));
    }
  }
}

/**
 * Submit handler for mapping form.
 */
function fundraiser_entity_default_mapping_form_submit($form, $form_state) {
  variable_set('fundraiser_entity_map_point', $form_state['values']['fundraiser_entity_map_point']);
  variable_set('fundraiser_entity_webform_autocreate_ignore', array_keys(array_filter($form_state['values']['fundraiser_entity_webform_autocreate_ignore'])));

  // Save the map values to the setting
  $mapped = array();

  foreach ($form_state['values']['mapping'] as $name => $values) {
    if (!empty($form['#donation_fields'][$name]) && !empty($values['source'])) {
      $mapped[$name] = array(
        'source' => $values['source'],
        'other' => $values['other'],
      );
    }
  }

  variable_set('fundraiser_entity_default_map', $mapped);
}

/**
 * Submit handler to redirect to the reset confirmation form
 */
function fundraiser_entity_default_mapping_form_reset($form, &$form_state) {
  $form_state['redirect'] = 'admin/springboard/settings/donation-entity/webform-map/reset';
}

/**
 * Confirmation form to reset to the default form
 */
function fundraiser_entity_default_mapping_reset_confirm($form, $form_state) {
  return confirm_form(
    array(),
    t('Are you sure you want to reset the default map?'),
    'admin/springboard/settings/donation-entity/webform-map',
    t('This action cannot be undone.')
  );
}

/**
 * Submit handler to reset to the default form
 */
function fundraiser_entity_default_mapping_reset_confirm_submit($form, &$form_state) {
  variable_set('fundraiser_entity_default_map', _fundraiser_entity_default_webform_map_array());
  drupal_set_message('The map has been reset to the default values.');
  $form_state['redirect'] = 'admin/springboard/settings/donation-entity/webform-map';
}

function _fundraiser_entity_load_all_webform_components() {
 // Load components for each node.
 return db_select('webform_component')
   ->distinct('')
   ->fields('webform_component', array('form_key', 'name'))
   ->condition('type', array('textfield', 'select', 'email', 'hidden'), 'IN')
   ->orderBy('name')
   ->execute()
   ->fetchAllKeyed();
}

/**
 * Returns the default autocreate ignore array
 */
function _fundraiser_entity_default_webform_ignore_array() {
  return array(
    'address',
    'address_line_2',
    'city',
    'country',
    'mail',
    'first_name',
    'last_name',
    'other_amount',
    'amount',
    'quantity',
    'recurs_monthly',
    'state',
    'zip',
  );
}

/**
 * Returns the default map array
 */
function _fundraiser_entity_default_webform_map_array() {
  return array(
    'field_fde_cc_exp_month' => array(
      'source' => 'other',
      'other' => '[donation:card_expiration_month]',
    ),
    'field_fde_cc_exp_year' => array(
      'source' => 'other',
      'other' => '[donation:card_expiration_year]',
    ),
    'field_fde_cc_number' => array(
      'source' => 'other',
      'other' => '[donation:card_number]',
    ),
    'field_fde_cc_type' => array(
      'source' => 'other',
      'other' => '[donation:card_type]',
    ),
    'field_fde_donor_address:country' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:country',
      'other' => '',
    ),
    'field_fde_donor_address:name_line' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:name_line',
      'other' => '',
    ),
    'field_fde_donor_address:first_name' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:first_name',
      'other' => '',
    ),
    'field_fde_donor_address:last_name' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:last_name',
      'other' => '',
    ),
    'field_fde_donor_address:organisation_name' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:organisation_name',
      'other' => '',
    ),
    'field_fde_donor_address:administrative_area' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:administrative_area',
      'other' => '',
    ),
    'field_fde_donor_address:sub_administrative_area' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:sub_administrative_area',
      'other' => '',
    ),
    'field_fde_donor_address:locality' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:locality',
      'other' => '',
    ),
    'field_fde_donor_address:dependent_locality' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:dependent_locality',
      'other' => '',
    ),
    'field_fde_donor_address:postal_code' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:postal_code',
      'other' => '',
    ),
    'field_fde_donor_address:thoroughfare' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:thoroughfare',
      'other' => '',
    ),
    'field_fde_donor_address:premise' => array(
      'source' => 'commerce_customer_profile--commerce_customer_address:premise',
      'other' => '',
    ),
    'field_fde_eft_account_number' => array(
      'source' => 'other',
      'other' => '[donation:bank_account]',
    ),
    'field_fde_eft_routing_number' => array(
      'source' => 'other',
      'other' => '[donation:bank_routing]',
    ),
    'field_fde_payment_method' => array(
      'source' => 'other',
      'other' => '[donation:payment_method]',
    ),
    'field_fde_user_agent' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-user_agent]',
    ),
    'field_fde_sustainer' => array(
      'source' => 'other',
      'other' => '[donation:recurs_monthly]',
    ),
    'field_fde_email' => array(
      'source' => 'user--mail',
      'other' => '',
    ),
    'field_fde_ms' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-ms]',
    ),
    'field_fde_cid' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-cid]',
    ),
    'field_fde_referrer' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-referrer]',
    ),
    'field_fde_initial_referrer' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-initial_referrer]',
    ),
    'field_fde_search_engine' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-search_engine]',
    ),
    'field_fde_search_string' => array(
      'source' => 'other',
      'other' => '[webform-tokens:val-search_string]',
    ),
    'field_fde_transaction_date' => array(
      'source' => 'commerce_payment_transaction--created',
      'other' => '',
    ),
  );
}
