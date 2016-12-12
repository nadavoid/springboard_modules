<?php

/**
 * @file
 * Administration.
 */

/**
 * Form callback for settings page.
 */
function salesforce_log_settings($form, $form_state) {
  $form['salesforce_log_batch_row_limit'] = array(
    '#type' => 'select',
    '#title' => t('Discard batch entries above the following row limit'),
    '#default_value' => variable_get('salesforce_log_batch_row_limit', 0),
    '#options' => drupal_map_assoc(array(
      '',
      5000,
      10000,
      25000,
      50000,
      75000,
      100000,
    )),
    '#description' => t('The maximum number of rows to keep in the batch log.'),
  );
  $form['salesforce_log_item_row_limit'] = array(
    '#type' => 'select',
    '#title' => t('Discard item entries above the following row limit'),
    '#default_value' => variable_get('salesforce_log_item_row_limit', 0),
    '#options' => drupal_map_assoc(array(
      '',
      5000,
      10000,
      25000,
      50000,
      75000,
      100000,
    )),
    '#description' => t('The maximum number of rows to keep in the item log.'),
  );
  $form = system_settings_form($form);

  return $form;
}

/**
 * Provide a single block from the administration menu as a page.
 *
 * @return string
 *   The output HTML.
 */
function salesforce_log_admin_menu_block_page() {
  $item = menu_get_item();
  if ($content = system_admin_menu_block($item)) {
    $output = theme('admin_block_content', array('content' => $content));
  }
  else {
    $output = t('You do not have any administrative items.');
  }
  return $output;
}
