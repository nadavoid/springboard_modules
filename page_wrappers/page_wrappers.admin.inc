<?php

/**
 * @file
 * Administration functions for page wrappers module.
 *
 * Provides admin functionality for the page wrappers module.
 */

/**
 * Admin configuration form for page wrappers.
 */
function page_wrappers_settings_form($form, $form_state) {
  $form['#attached']['js'][] = drupal_get_path('module', 'page_wrappers') . '/js/page_wrappers.admin.js';
  $types = node_type_get_types();
  unset($types['page_wrapper']);
  $options = array();
  foreach ($types as $db_name => $type) {
    $options[$db_name] = $type->name;
  }

  $form['page_wrapper_enabled_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Page wrapper-enabled content types'),
    '#options' => $options,
    '#default_value' => variable_get('page_wrapper_enabled_node_types', array('donation_form')),
    '#description' => t('A list of page wrappers will be available on these content types.'),
  );

  $wrappers = _page_wrappers_available_wrappers();
  $rulesets = _page_wrapper_available_rulesets();

  // To use the same ajax function as the node form we check for the node value
  if (!empty($form_state['node']->page_wrappers)) {
    $configurations = $form_state['node']->page_wrappers;
  }
  else {
    $configurations = array_values(variable_get('page_wrapper_default_node_config', array()));
  }

  // Get a count of the wrappers, the wrappers_count value is passed from the ajax function
  if (isset($form_state['wrappers_count'])) {
    $wrappers_count = $form_state['wrappers_count'];
  }
  else {
    $wrappers_count = empty($configurations) ? 1 : count($configurations);
  }

  $form['page_wrappers_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default Node Configuration'),
    '#description' => t('Configure the Page Wrapper settings for a new node.'),
    '#tree' => FALSE,
  );

  // Container for just the poll choices.
  $form['page_wrappers_settings']['page_wrapper_default_node_config'] = array(
    '#prefix' => '<div id="page-wrappers-assignments">',
    '#suffix' => '</div>',
    '#theme' => 'page_wrappers_node_assignments',
  );

  // Let the ajax functions know which field group to update
  $form['#page_wrapper_config_parent'] = 'page_wrapper_default_node_config';

  // Add existing wrapper configurations
  $delta = 0;
  $weight = 0;
  if (!empty($configurations)) {
    $delta = count($configurations);
    foreach ($configurations as $pwid => $config) {
      $key = 'pwid:' . $pwid;
      $form['page_wrappers_settings']['page_wrapper_default_node_config'][$key] = _page_wrappers_assignment_fields($key, $config, $wrappers, $rulesets, $config['weight'], $wrappers_count, 'page_wrapper_default_node_config');
      $weight = max($config['weight'], $weight);
    }
  }

  // Add more
  $existing_delta = $delta;
  for ($delta; $delta < $wrappers_count; $delta++) {
    $key = 'new:' . ($delta - $existing_delta);
    $weight++;
    $form['page_wrappers_settings']['page_wrapper_default_node_config'][$key] = _page_wrappers_assignment_fields($key, array(), $wrappers, $rulesets, $weight, $wrappers_count, 'page_wrapper_default_node_config');
  }

  // If we have configured rulesets add the add more button
  if (!empty($rulesets)) {
    $form['page_wrappers_settings']['page_wrappers_more'] = array(
      '#type' => 'submit',
      '#value' => t('Add Another'),
      '#weight' => 1,
      '#limit_validation_errors' => array(array('page_wrapper_default_node_config')),
      '#submit' => array('page_wrapper_add_assignment_submit'),
      '#ajax' => array(
        'callback' => '_page_wrappers_add_js',
        'wrapper' => 'page-wrappers-assignments',
        'effect' => 'fade',
      ),
    );
    // Add a flag to the page wrapper fieldset that we have rulesets
    $form['page_wrappers_settings']['page_wrapper_default_node_config']['#has_rulesets'] = TRUE;
  }
  
  // Settings for caching on page wrapper node operations.
  $form['page_wrappers_cache_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Cache settings'),
    '#tree' => FALSE,
  );
  $form['page_wrappers_cache_settings']['page_wrappers_skip_full_cache_clear'] = array(
    '#type' => 'checkbox',
    '#title' => t('Bypass drupal_flush_all_caches().'),
    '#description' => t('Enabling this option will use a subset of drupal_flush_all_caches() routines during page wrapper node operations.'),
    '#default_value' => variable_get('page_wrappers_skip_full_cache_clear', FALSE),
  );
  // Get a list of all potential contrib cache tables.
  $hook_cache_tables = module_invoke_all('flush_caches');
  $form['page_wrappers_cache_settings']['page_wrapper_cache_tables_exclude'] = array(
    '#type' => 'checkboxes',
    '#states' => array(
      'visible' => array(
        ':input[name="page_wrappers_skip_full_cache_clear"]' => array('checked' => TRUE),
      ),
    ),
    '#title' => t('Exclude cache tables.'),
    '#options' => drupal_map_assoc($hook_cache_tables),
    '#default_value' => variable_get('page_wrapper_cache_tables_exclude', array()),
    '#description' => t('Cache tables selected above <b>will not be flushed</b> during page wrapper node operations.'),
  );

  $form['#validate'][] = 'page_wrappers_settings_form_validate';

  return system_settings_form($form);
}

/**
 * Validate handler for the admin form to remove empty page wrapper default node configurations
 */
function page_wrappers_settings_form_validate($form, &$form_state) {
  foreach ($form_state['values']['page_wrapper_default_node_config'] as $key => $values) {
    if (empty($values['wrapper_nid'])) {
      unset($form_state['values']['page_wrapper_default_node_config'][$key]);
    }
  }
}
