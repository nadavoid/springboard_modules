<?php

/**
 * @file
 * Functions for the shopping cart component.
 */

/**
 * Default values for designation form.
 *
 * @return array
 *   Component defaults.
 */
function _webform_defaults_fd_cart() {
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
function _webform_edit_fd_cart($component) {
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
function _webform_submit_fd_cart($component, $values) {
  $return = array();
  foreach ($values as $key => $submitted_value) {
    $return[] = serialize(array($key => $submitted_value));
  }
  return $return;
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_fd_cart($component, $value = NULL, $filter = TRUE) {

  $element = array(
    '#type' => 'item',
    '#title' => $filter ? _webform_filter_xss($component['name']) : $component['name'],
    '#weight' => $component['weight'],
    '#attributes' => array('class' => array('webform-component-designations')),
    '#title_display' => 'none',
    '#webform_component' => $component,
    '#theme' => 'fundraiser_designations_fund_cart_component',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'fundraiser_designations') . '/js/fundraiser_designations.js'),
      'css' => array(drupal_get_path('module', 'fundraiser_designations') . '/css/designation-client-form.css'),
    ),
  );

  $item = array();
  $item['#id'] = 'cart_body';
  $item['#type'] = 'item';
  $element[$item['#id']] = $item;

  $child = array();
  $child['#type'] = 'textfield';
  $child['#id'] = 'cart_total';
  $child['#title'] = 'Total';
  $child['#disabled'] = TRUE;
  $child['#title_display'] = 'none';
  $element[$item['#id']][$child['#id']] = $child;

  $child = array();
  $child['#type'] = 'hidden';
  $child['#id'] = 'fund_catcher';
  $child['#title'] = '';
  $child['#title_display'] = 'none';
  $element[$item['#id']][$child['#id']] = $child;

  return $element;
}

/**
 * Pre-render function to add our id to the js settings.
 */
function webform_designations_cart_prerender($element) {
  $id = 'webform-component-' . str_replace('_', '-', implode('--', array_slice($element['#parents'], 1)));
  $element['#attributes']['id'] = $id;
  drupal_add_js(array('fundraiserDesignationsCart' => array('table' => array('id' => $id))), 'setting');

  return $element;
}

/**
 * Theme the form elements into a table.
 */
function theme_fundraiser_designations_fund_cart_component($vars) {

  $element = $vars['element'];

  $output = '';
  $build['content'] = array(
    '#prefix' => '<div class="fundraiser-designation-cart-wrapper">',
    '#suffix' => '</div>',
    'fund_cart_table' => array(
      '#type' => 'markup',
      '#markup' => fundraiser_designations_cart_table($element),
      '#prefix' => '<div class="fundraiser-designation-cart-table-wrapper">',
      '#suffix' => '</div>',
      '#weight' => -6,
    ),
    'fund_catcher' => array(
      '#type' => 'markup',
      '#markup' => drupal_render($element['cart_body']['fund_catcher']),
      '#prefix' => '<div id="fund-catcher">',
      '#suffix' => '</div>',
      '#weight' => -6,
    ),
  );
  $output .= drupal_render($build);

  return $output;
}

function fundraiser_designations_cart_table($element) {
  $header = array(
    '',
    'Your Fund Selections',
    'Amount',
  );

  $rows = [];
  $rows[] = array(
    'data' => array(
      array(
        'data' => '&nbsp;',
      ),
      array(
        'data' => t('No funds selected'),
        'class' => array('fund-name'),
      ),
      array(
        'data' => '&nbsp;',
        'class' => array('fund-amount-empty'),
      ),
    ),
    'class' => array('cart-fund-row', 'cart-fund-empty'),
  );
  $rows[] = array(
    'data' => array(
      array(
        'data' => check_plain($element['cart_body']['cart_total']['#title']),
        'colspan' => 2,
      ),
      array(
        'data'  => drupal_render($element['cart_body']['cart_total']),
        'class' => array('cart-total-cell'),
      ),
    ),
    'class' => array('cart-total-row'),
  );

  $table = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array(
      'class' => array('fundraiser-designation-cart-table'),
    ),
  );
  return drupal_render($table);
}