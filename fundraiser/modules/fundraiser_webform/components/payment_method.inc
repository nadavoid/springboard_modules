<?php

/**
 * @file
 * Webform module payment_method component.
 */

// Payment methods element depends on functions provided by select.
webform_component_include('select');

/**
 * Implements _webform_defaults_component().
 */
function _webform_defaults_payment_method() {
  return array(
    'name' => '',
    'form_key' => 'payment_method',
    'mandatory' => 1,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'extra' => array(
      'items' => '',
      'aslist' => 0,
      'multiple' => 0,
      'optrand' => 0,
      'other_option' => NULL,
      'other_text' => NULL,
      'title_display' => 0,
      'description' => '',
      'custom_keys' => FALSE,
      'options_source' => '',
      'private' => FALSE,
      'payment_methods' => array(),
    ),
  );
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_payment_method($component) {
  // Use the standard fields for the select
  $form = _webform_edit_select($component);
  // Remove some items we don't need
  $form['value']['#access'] = FALSE;
  $form['display']['#access'] = FALSE;
  $form['extra']['description']['#access'] = FALSE;
  $form['extra']['multiple']['#access'] = FALSE;
  $form['extra']['items']['#access'] = FALSE;
  $form['extra']['options_source']['#access'] = FALSE;

  // Initialize the fieldset
  $fieldset = array(
    '#type' => 'fieldset',
    '#title' => t('Payment methods'),
    '#description' => t('Enable donation form payment methods and their corresponding gateways.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#theme' => 'fundraiser_webform_component_payment_methods',
  );

  // Get all available payment methods.
  $temp_gateways = _fundraiser_gateway_info();
  if (empty($temp_gateways)) {
    $form['no_gateways_notice']['#markup'] = '<div>' . t('There are no gateways configured. Please create some gateways in the store configuration.') . '</div>';
    return $form;
  }
  foreach ($temp_gateways as $temp_gateway) {
    $types = isset($temp_gateway['payment_method']) ? $temp_gateway['payment_method'] : array();
    foreach ($types as $type) {
      $all_payment_methods[$type][] = $temp_gateway;
    }
  }
  // Sort the payment gateways from the existing settings of the field.
  $field_payment_methods = !empty($component['extra']['payment_methods']) ? $component['extra']['payment_methods'] : array();
  foreach (array_keys($all_payment_methods) as $method_key) {
    $all_payment_methods[$method_key]['weight'] = isset($field_payment_methods[$method_key]['weight']) ? $field_payment_methods[$method_key]['weight'] : 0;
  }
  // Sort all payment methods by weight.
  uasort($all_payment_methods, '_fundraiser_webform_payment_method_weight_compare');

  // Loop through each method
  foreach ($all_payment_methods as $method_key => $method) {
    // Remove the weight value
    unset($method['weight']);
    $field_method_settings = isset($field_payment_methods[$method_key]) ? $field_payment_methods[$method_key] : array();
    // Build the #options array for the payment processors supporting this payment method.
    $processor_options = array(NULL => '- ' . t('select') . ' -');
    foreach (element_children($method) as $key) {
      $processor_options[$method[$key]['id']] = $method[$key]['title'];
    }
    $fieldset[$method_key] = array('#tree' => TRUE);
    $fieldset[$method_key]['status'] = array(
      '#type' => 'checkbox',
      '#default_value' => isset($field_method_settings['status']) ? $field_method_settings['status'] : 0,
    );
    $fieldset[$method_key]['_status'] = array(
      '#type' => 'markup',
      '#title' => t('Method'),
      '#markup' => $method_key, // Could use a pretty name here
    );
    $fieldset[$method_key]['method'] = array(
      '#type' => 'value',
      '#value' => $method_key,
    );
    $fieldset[$method_key]['id'] = array(
      '#type' => 'select',
      '#title' => t('Payment Processor'),
      '#options' => $processor_options,
      '#default_value' => isset($field_method_settings['id']) ? $field_method_settings['id'] : NULL,
    );
    $fieldset[$method_key]['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#size' => 20,
      '#default_value' => isset($field_method_settings['label']) ? $field_method_settings['label'] : NULL,
    );
    $fieldset[$method_key]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#delta' => 25,
      '#default_value' => isset($field_method_settings['weight']) ? $field_method_settings['weight'] : 0,
      '#attributes' => array('class' => array('fundraiser-webform-payment-methods-weight')),
    );
    $fieldset[$method_key]['_default'] = array(
      '#type' => 'radio',
      '#title' => t('Default'),
      '#default_value' => !empty($field_method_settings['default']) ? $method_key : NULL,
      '#return_value' => $method_key,
      '#name' => 'extra[payment_methods][_default]',
      '#parents' => array('extra', 'payment_methods', '_default'),
    );
  }

  $form['extra']['payment_methods'] = $fieldset;
  // add a validate callback.
  $form['#validate'][] = '_fundraiser_webform_payment_method_validate';
  return $form;
}

/**
 * Validate handler for our component, also sets the default value
 */
function _fundraiser_webform_payment_method_validate($form, &$form_state) {
  $values = $form_state['values']['extra']['payment_methods'];

  // Capture the default method and unset that value
  $default_method = $values['_default'];
  unset($form_state['values']['extra']['payment_methods']['_default']);

  // Filter the values array to just enabled methods.
  $enabled_methods = array_filter($values, '_fundraiser_webform_payment_method_array_filter_enabled');

  // Check that there is at least one method enabled
  if (empty($enabled_methods)) {
    form_set_error('payment_methods', t('At least one payment method must be enabled.'));
  }
  else {
    // Check whether the default method is one of the enabled methods.
    if (!isset($enabled_methods[$default_method])) {
      if (count($enabled_methods) == 1) {
        // If there's only one enabled method, skip validating _default_method
        // and just change the value. Stop wasting people's time.
        $default_method = key($enabled_methods);
        $form_state['values']['extra']['payment_methods'][$default_method]['default'] = 1;
      }
      else {
        form_set_error('extra[payment_methods][_default', t('Default method must be one of the enabled methods.'));
      }
    }
    else {
      // Set the default value to the appropriate method
      // Having to do this here since a submit handler fires after the component is saved
      $form_state['values']['extra']['payment_methods'][$default_method]['default'] = 1;
    }
  }
}

/**
 * Theme function for the payment methods table on the component edit form.
 */
function theme_fundraiser_webform_component_payment_methods($variables) {
  $fieldset = $variables['form'];
  $header = array();
  $rows = array();

  foreach (element_children($fieldset) as $i) {
    $row = array();
    foreach (element_children($fieldset[$i]) as $field) {
      $this_element = $fieldset[$i][$field];

      if ($this_element['#type'] != 'value') {
        if (!isset($header[$field])) {
          $header[$field] = isset($this_element['#title']) ? $this_element['#title'] : '';
        }
        unset($this_element['#title']);
        $row[$field] = drupal_render($this_element);
      }
    }

    $rows[] = array(
      'data' => $row,
      'class' => array('draggable'),
    );
  }

  // Adding tabledrag support to this table.
  drupal_add_tabledrag('fundraiser-webform-payment-methods', 'order', 'sibling', 'fundraiser-webform-payment-methods-weight');

  return theme('table', array('rows' => $rows, 'header' => $header, 'attributes' => array('id' => 'fundraiser-webform-payment-methods')));
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_payment_method($component, $value = NULL, $filter = TRUE) {
  // Use the default select list function to build the element
  $element = _webform_render_select($component, $value, $filter);
  // Add custom post render function before the final form render
  $element['#post_render'] = array('_fundraiser_webform_component_payment_method_post_render');
  // Add a class for states interaction with payment fields
  $element['#attributes'] = array(
    'class' => array('fundraiser-payment-methods'),
  );
  if (!empty($component['extra']['payment_methods'])) {
    // Use the enabled methods
    $payment_methods = array_filter($component['extra']['payment_methods'], '_fundraiser_webform_payment_method_array_filter_enabled');
    // Sort all payment methods by weight.
    uasort($payment_methods, '_fundraiser_webform_payment_method_weight_compare');
    $node = isset($component['nid']) ? node_load($component['nid']) : NULL;
    drupal_alter('fundraiser_webform_payment_method_component_methods', $payment_methods, $node);
    // Loop through the payment methods to create our options and find the default
    $default = NULL;
    // If there is only one method this field is hidden
    if (count($payment_methods) == 1) {
      reset($payment_methods);
      $element['#type'] = 'hidden';
      $element['#value'] = key($payment_methods);
      // Remove the radio theme function and title
      $element['#theme_wrappers'] = array();
      $element['#title'] = '';
    }
    else {
      foreach ($payment_methods as $payment_method) {
        // Set the option label as the custom label or the hard-coded label
        $options[$payment_method['method']] = !empty($payment_method['label']) ? $payment_method['label'] : $payment_method['method'];
        $default = !empty($payment_method['default']) ? $payment_method['method'] : $default;
        $element['#options'] = $options;
        $element['#default_value'] = $default;
      }
    }
  }
  return $element;
}

/**
 * Sort payment methods by weight
 */
function _fundraiser_webform_payment_method_weight_compare($a, $b) {
  $weight = $a['weight'] - $b['weight'];
  return $weight;
}

/**
 * Post-render function to add the id and active payment methods to the js settings
 */
function _fundraiser_webform_component_payment_method_post_render($content, $element) {
  // Add the field id to the js settings
  drupal_add_js(array('fundraiserWebform' => array('components' => array('payment_method' => array('id' => $element['#id'])))), 'setting');
  // Add the active methods to js settings
  $delta = 1;
  $js_settings = array();
  foreach ($element['#options'] as $key => $value) {
    $js_settings[$delta++] = $key;
  }
  drupal_add_js(array('fundraiserWebform' => array('active_methods' => $js_settings)), 'setting');
  return $content;
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_payment_method($component, $value, $format = 'html') {
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme_wrappers' => $format == 'html' ? array('webform_element', 'webform_element_wrapper') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#format' => $format,
    '#options' => _webform_select_options($component, !$component['extra']['aslist']),
    '#value' => (array) $value,
    '#translatable' => array('title', 'options'),
    '#webform_component' => $component,
  );
}

/**
 * Implements _webform_submit_component().
 *
 * Convert FAPI 0/1 values into something saveable.
 */
function _webform_submit_payment_method($component, $value) {
  return _webform_submit_select($component, $value);
}

/**
 * Implements _webform_analysis_component().
 */
function _webform_analysis_payment_method($component, $sids = array(), $single = FALSE) {
  $query = db_select('webform_submitted_data', 'wsd', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('wsd', array('data'))
    ->condition('nid', $component['nid'])
    ->condition('cid', $component['cid'])
    ->condition('data', '', '<>')
    ->groupBy('data');
  $query->addExpression('COUNT(data)', 'datacount');

  if (count($sids)) {
    $query->condition('sid', $sids, 'IN');
  }

  $result = $query->execute();
  $rows = array();
  foreach ($result as $data) {
    $display_option = $data['data'];
    $rows[$data['data']] = array(_webform_filter_xss($display_option), $data['datacount']);
  }

  return $rows;

}

/**
 * Implements _webform_table_component().
 */
function _webform_table_payment_method($component, $value) {
  return _webform_table_select($component, $value);
}

/**
 * Implements _webform_csv_headers_component().
 */
function _webform_csv_headers_payment_method($component, $export_options) {
  return _webform_csv_headers_select($component, $export_options);
}

/**
 * Implements _webform_csv_data_component().
 */
function _webform_csv_data_payment_method($component, $export_options, $value) {
  return _webform_csv_data_select($component, $export_options, $value);
}
