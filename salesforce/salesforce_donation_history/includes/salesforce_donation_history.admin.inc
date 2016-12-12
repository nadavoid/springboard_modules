<?php
/**
 * @file
 * Admin form definitions for salesforce_donation_history.
 */

/**
 * Page callback.
 */
function salesforce_donation_history_query() {
  $form = array();
  $form['salesforce_donation_history_query'] = array(
    '#type' => 'textarea',
    '#title' => t('Opportunities query'),
    '#default_value' => variable_get('salesforce_donation_history_query', SALESFORCE_DONATION_HISTORY_DEFAULT_SOQL),
    '#description' => "Enter the Salesforce SOQL query used to get opportunities. <ul><li>The Opportunity fields <samp>StageName</samp>, <samp>Id</samp> and <samp>CloseDate</samp> are required.</li><li>The query must end with a <samp>WHERE</samp> clause.</li><li>Do not include a <samp>LIMIT</samp> clause.</li>",
  );
  $form['salesforce_donation_history_stage_name'] = array(
    '#type' => 'textarea',
    '#title' => t('StageName values included'),
    '#default_value' => variable_get('salesforce_donation_history_stage_name', SALESFORCE_DONATION_HISTORY_DEFAULT_STAGE_NAME),
    '#description' => "Enter the values for the StageName field for the Opportunities you wish to include in display, e.g. <samp>Posted</samp> and <samp>Partially Refunded</samp>. One value per line. Values are not case sensitive. This is used to prevent inappropriate opportunities (like pledged or refunded, for example) from being displayed on the donation history page or the annual tax summary receipt.",
  );
  $form['#validate'][] = 'salesforce_donation_history_query_validate';
  $form = system_settings_form($form);
  return $form;
}

/**
 * Page callback.
 */
function salesforce_donation_history_fields() {
  $form = array();
  $field_definitions = variable_get('salesforce_donation_history_query_fields', NULL);
  $field_defaults = variable_get('salesforce_donation_history_field_settings', NULL);
  $form['salesforce_donation_history_field_settings'] = array(
    '#prefix' => '<div id="salesforce_donation_history_field_settings">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#theme' => 'salesforce_donation_history_admin_field_table',
  );
  foreach ($field_definitions as $sf_table => $sf_field) {
    $form['salesforce_donation_history_field_settings'][$sf_table]['label'] = array(
      '#type' => 'textfield',
      '#default_value' => !empty($field_defaults[$sf_table]['label']) ? $field_defaults[$sf_table]['label'] : NULL,
      '#extra_data' => array(
        'field_table' => $sf_table,
        'field_name' => '',
      ),
    );
    $form['salesforce_donation_history_field_settings'][$sf_table]['show_on_tab'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($field_defaults[$sf_table]['show_on_tab']) ?: 0,
    );
    $form['salesforce_donation_history_field_settings'][$sf_table]['show_on_receipt'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($field_defaults[$sf_table]['show_on_receipt']) ?: 0,
    );
    $form['salesforce_donation_history_field_settings'][$sf_table]['show_on_summary'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($field_defaults[$sf_table]['show_on_summary']) ?: 0,
    );
    foreach ($sf_field as $name => $value) {
      $form['salesforce_donation_history_field_settings'][$sf_table . '-' . $name]['label'] = array(
        '#type' => 'textfield',
        '#default_value' => !empty($field_defaults[$sf_table . '-' . $name]['label']) ? $field_defaults[$sf_table . '-' . $name]['label'] : NULL,
        '#extra_data' => array(
          'field_table' => $sf_table,
          'field_name' => $name,
        ),
      );
      $form['salesforce_donation_history_field_settings'][$sf_table . '-' . $name]['show_on_tab'] = array(
        '#type' => 'checkbox',
        '#default_value' => !empty($field_defaults[$sf_table . '-' . $name]['show_on_tab']) ?: 0,
      );
      $form['salesforce_donation_history_field_settings'][$sf_table . '-' . $name]['show_on_receipt'] = array(
        '#type' => 'checkbox',
        '#default_value' => !empty($field_defaults[$sf_table . '-' . $name]['show_on_receipt']) ?: 0,
      );
      $form['salesforce_donation_history_field_settings'][$sf_table . '-' . $name]['show_on_summary'] = array(
        '#type' => 'checkbox',
        '#default_value' => !empty($field_defaults[$sf_table . '-' . $name]['show_on_summary']) ?: 0,
      );
    }
  }
  $form = system_settings_form($form);
  return $form;
}

/**
 * Page callback for annual summary admin form.
 */
function salesforce_donation_history_annual_summary() {
  $form = array();
  $defaults = array(
    'value' => '',
    'format' => filter_default_format(),
  );
  $form['description'] = array(
    '#type' => 'markup',
    '#markup' => t('The header and footer text fields will appear above and below a table containing the donation information on the receipt. You can control which columns are displayed in this table on the <a href="fields">Fields</a> tab.'),
  );
  $salesforce_donation_history_annual_summary_intro = variable_get('salesforce_donation_history_annual_summary_intro', $defaults);
  $form['salesforce_donation_history_annual_summary_intro'] = array(
    '#type' => 'text_format',
    '#title' => t('Header text'),
    '#default_value' => $salesforce_donation_history_annual_summary_intro['value'],
    '#description' => 'This will appear above the table containing donation information.',
    '#format' => $salesforce_donation_history_annual_summary_intro['format'],
    '#element_validate' => array('token_element_validate'),
    '#token_types' => array('user'),
  );
  $salesforce_donation_history_annual_summary_outro = variable_get('salesforce_donation_history_annual_summary_outro', $defaults);
  $form['salesforce_donation_history_annual_summary_outro'] = array(
    '#type' => 'text_format',
    '#title' => t('Footer text'),
    '#default_value' => $salesforce_donation_history_annual_summary_outro['value'],
    '#description' => 'This will appear below the table containing donation formation.',
    '#format' => $salesforce_donation_history_annual_summary_outro['format'],
    '#element_validate' => array('token_element_validate'),
    '#token_types' => array('user'),
  );
  $form['token_help'] = array(
    '#theme' => 'token_tree_link',
    '#token_types' => array('user'),
  );
  $form = system_settings_form($form);
  return $form;
}

/**
 * Page callback for individual donation admin form.
 */
function salesforce_donation_history_donation() {
  $form = array();
  $defaults = array(
    'value' => '',
    'format' => filter_default_format(),
  );
  $form['description'] = array(
    '#type' => 'markup',
    '#markup' => t('The header and footer text fields will appear above and below a table containing the donation information on the receipt. You can control which columns are displayed in this table on the <a href="fields">Fields</a> tab.'),
  );
  $salesforce_donation_history_donation_intro = variable_get('salesforce_donation_history_donation_intro', $defaults);
  $form['salesforce_donation_history_donation_intro'] = array(
    '#type' => 'text_format',
    '#title' => t('Header text'),
    '#default_value' => $salesforce_donation_history_donation_intro['value'],
    '#description' => 'This will appear above the table containing donation information.',
    '#format' => $salesforce_donation_history_donation_intro['format'],
    '#element_validate' => array('token_element_validate'),
    '#token_types' => array('user'),
  );
  $salesforce_donation_history_donation_outro = variable_get('salesforce_donation_history_donation_outro', $defaults);
  $form['salesforce_donation_history_donation_outro'] = array(
    '#type' => 'text_format',
    '#title' => t('Footer text'),
    '#default_value' => $salesforce_donation_history_donation_outro['value'],
    '#description' => 'This will appear below the table containing donation formation.',
    '#format' => $salesforce_donation_history_donation_outro['format'],
    '#element_validate' => array('token_element_validate'),
    '#token_types' => array('user'),
  );
  $form['token_help'] = array(
    '#theme' => 'token_tree_link',
    '#token_types' => array('user'),
  );
  $form = system_settings_form($form);
  return $form;
}


/**
 * Validation handler for salesforce_donation_history_query form.
 */
function salesforce_donation_history_query_validate($form, &$form_state) {
  $result = salesforce_donation_history_test_query($form_state['input']['salesforce_donation_history_query']);
  if ($result->size != 1) {
    form_set_error('salesforce_donation_history_query', t('There was a problem performing this query. A sample record could not be found.'));
  }
  else {
    $query_fields = salesforce_donation_history_get_query_fields($result->records);
    $field_types = salesforce_donation_history_get_field_types($query_fields);
    variable_set('salesforce_donation_history_query_fields', $field_types);
    cache_clear_all('*', 'cache_salesforce_donation_history', TRUE);
    drupal_set_message('Please review the field configuration options below.');
    $form_state['redirect'] = array('admin/config/salesforce/donation_history/fields');
  }
}
