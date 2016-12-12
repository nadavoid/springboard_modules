<?php

/**
 * @file
 * Designation component functions.
 */

/**
 * Default values for designation form.
 *
 * @return array
 *   Component defaults.
 */
function _webform_defaults_fd_addon() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'extra' => array(
      'field_prefix' => '',
      'field_suffix' => '',
    ),
  );
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_fd_addon($component) {
  $form = array();
  $form['edit_notice']['#markup'] = '<div>' . t('The settings for this field are controlled by Fundraiser.') . '</div>';
  $form['display']['#access'] = FALSE;
  return $form;
}

/**
 * Implements _webform_submit_component().
 *
 * Convert submitted keys and values into serialized, saveable data.
 */
function _webform_submit_fd_addon($component, $values) {
  $return = array();
  foreach ($values as $key => $submitted_value) {
    $return[] = serialize(array($key => $submitted_value));
  }
  return $return;
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_fd_addon($component, $value = NULL, $filter = TRUE) {

  $element = array(
    '#type' => 'item',
    '#title' => $filter ? _webform_filter_xss($component['name']) : $component['name'],
    '#weight' => $component['weight'],
    '#attributes' => array('class' => array('webform-component-addon')),
    '#title_display' => 'none',
    '#webform_component' => $component,
    '#theme' => 'fundraiser_designations_fund_addon_component',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'fundraiser_designations') . '/js/fundraiser_designations.js'),
      'css' => array(drupal_get_path('module', 'fundraiser_designations') . '/css/designation-client-form.css'),
    ),
  );

  $node = isset($component['nid']) ? node_load($component['nid']) : NULL;

  $data = !empty($node->fd_addon) ? $node->fd_addon : array();
  if (fundraiser_designations_is_designation_type($node->type) && !empty($data)) {

    // Load the addon data.
    $price_set = [];
    $fund = commerce_product_load($data['pid']);
    if (!empty($data['price_set'])) {
      $amounts = unserialize($data['price_set']);
      foreach ($amounts as $amount) {
        $price_set[] = array($amount['amount'], $amount['label']);
      }
    }
    $amounts = $price_set;
    _webform_render_addon_items($element, $node, $fund, $amounts);
  }
  return $element;
}

/**
 * Render the component when it has a single fund.
 *
 * @param array $element
 *   The webform component.
 * @param object $node
 *   The node.
 * @param object $group_wrapper
 *   The fund group.
 * @param object $funds
 *   The funds in the fund group.
 * @param array $amounts
 *   The one-time and/or recurring price sets.
 * @param array $groups_by_weight
 *   The fund groups on this form.
 */
function _webform_render_addon_items(&$element, $node, $fund, $amounts) {
  $item = array();
  $item['#id'] = 'addon-item-' . $fund->product_id;
  $item['#extra']['addon_id'] = $fund->product_id;
  $item['#extra']['fund_id'] = $fund->product_id;
  $item['#attributes'] = array('class' => $fund->product_id);

  $item['#type'] = 'item';
  $element[$fund->product_id] = $item;

  $child = array();
  $child['#type'] = 'item';
  $child['#id'] = 'funds-placeholder-' . $fund->product_id;
  $child['#title'] = ' ';
  $element[$fund->product_id]['funds_placeholder_' . $fund->product_id] = $child;

  _fundraiser_designations_addon_build_child_elements($fund->product_id, $element, $amounts);

}

/**
 * @param int $id
 *   A group entity id.
 * @param array $element
 *   The parent webform component.
 * @param array $amounts
 *   The one-time and/or recurring price sets.
 */
function _fundraiser_designations_addon_build_child_elements($id, &$element, $amounts) {
  $donation_amounts = [];
  if (!empty($amounts)) {
    foreach ($amounts as $key => $amount) {
      $donation_amounts[$amount[0]] = check_plain($amount[1]);
    }
    $child = array();
    $child['#type'] = 'radios';
    $child['#id'] = 'default-amounts-' . $id;
    $child['#extra']['addon_id'] = $id;
    $child['#options'] = $donation_amounts;
    $child['#title'] = '';
    $element[$id]['default_amounts_' . $id] = $child;
  }
}

/**
 * Theme the form elements into a table.
 */
function theme_fundraiser_designations_fund_addon_component($variables) {
  $element = $variables['element'];
  $output = '';
  foreach (element_children($element) as $id) {
    $build = array();
    if (isset($element[$id]['#extra']['addon_id'])) {

      $build['content'] = array(
        '#prefix' => '<div class="designation-addon-wrapper" id="designation-addon-' . $element[$id]['#extra']['addon_id'] . '">',
        '#suffix' => '</div>',

        'fund_addoon_table' => array(
          '#type' => 'markup',
          '#markup' => fundraiser_designations_addon_table($element[$id]),
          '#prefix' => '<div class="designation-addon-funds-table-wrapper">',
          '#suffix' => '</div>',
          '#weight' => -6,
        ),
      );

      $output .= drupal_render($build);
    }
  }

  return $output;
}

/**
 * Build a table display for each fund group.
 *
 * @param array $element
 *   The fund group component.
 *
 * @return string
 *   The rendered table.
 */
function fundraiser_designations_addon_table($element) {
  $row[] = array(
    'data' => array(
      array(
        'data' => drupal_render($element),
        'align' => 'right',
      ),
    ),
  );

  if (isset($element['#extra']['fund_id'])) {
    $row[0]['data-placeholder-fund-id'] = $element['#extra']['fund_id'];
  }

  $table = array(
    '#theme' => 'table',
    '#header' => array(),
    '#rows' => $row,
    '#attributes' => array(
      'class' => array('designation-addon-funds-table'),
    ),
  );
  return drupal_render($table);
}
