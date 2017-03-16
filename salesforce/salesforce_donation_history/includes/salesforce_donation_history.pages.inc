<?php

/**
 * @file
 *  Page callbacks for donation history.
 */

/**
 * Page callback.
 */
function salesforce_donation_history_page($user) {
  drupal_add_css(drupal_get_path('module', 'salesforce_donation_history') . '/salesforce_donation_history.css');
  drupal_set_title('My Donation History');
  $output = array();
  $results = array();
  $params = drupal_get_query_parameters();
  $sfid = salesforce_donation_history_user_account_id($user->uid);
  if ($sfid) {
    $results = salesforce_donation_history_get_opportunities($sfid);
    $results = salesforce_donation_history_filter_by_status($results);
    $results = salesforce_donation_history_filter_by_date($results);
    $results = salesforce_donation_history_sort_by_year($results);
  }
  // Check to see if there are pending donations in the salesforce sync queue.
  if ($pending_count = salesforce_donation_history_count_pending_donations($user->uid)) {
    drupal_set_message(t('You have donation(s) in-process. Donations and refunds may take up to 30 minutes to appear here.'));
  }
  if (count($results) > 0) {
    // @TODO what happens when we default to displaying the current year but
    // there are only donations from previous years?
    $selected_year = isset($params['year']) ? $params['year'] : date('Y');
    $available_years = array_keys($results);
    $output['select-year'] = drupal_get_form('salesforce_donation_history_select_year', $available_years, $selected_year);
    $output['annual-tax-receipt'] = array(
      '#type' => 'markup',
      '#prefix' => '<div id="print-annual-tax-receipt">',
      '#suffix' => '</div>',
      '#markup' => l(t('Print Annual Tax Receipt'), 'user/' . $user->uid . '/donation-history/summary/' . $selected_year),
    );
    $output['table'] = salesforce_donation_history_create_table($results[$selected_year]);
  }
  else {
    $output['no-donations'] = array(
      '#type' => 'markup',
      '#markup' => t('No donations found.'),
    );
  }
  return $output;
}

/**
 * Page callback for annual tax summary.
 */
function salesforce_donation_history_annual_summary_page($user, $year) {
  drupal_add_css(drupal_get_path('module', 'salesforce_donation_history') . '/salesforce_donation_history.css');
  drupal_set_title(t('Annual Tax Receipt for :year', array(':year' => $year)), PASS_THROUGH);
  $output = array();
  $intro = variable_get('salesforce_donation_history_annual_summary_intro', NULL);
  $output['intro'] = array(
    '#type' => 'markup',
    '#markup' => token_replace(check_markup($intro['value'], $intro['format']), array('user' => $user)),
  );
  $sfid = salesforce_donation_history_user_account_id($user->uid);
  if ($sfid) {
    $results = salesforce_donation_history_get_opportunities($sfid);
    $results = salesforce_donation_history_filter_by_status($results);
    $results = salesforce_donation_history_filter_by_date($results);
    $results = salesforce_donation_history_sort_by_year($results);
  }
  if (count($results) > 0) {
    $output['table'] = salesforce_donation_history_create_table($results[$year], 'show_on_summary', TRUE, FALSE);
  }
  else {
    $output['no-donations'] = array(
      '#type' => 'markup',
      '#markup' => t('No donations found.'),
    );
  }
  $outro = variable_get('salesforce_donation_history_annual_summary_outro', NULL);
  $output['footer'] = array(
    '#type' => 'markup',
    '#markup' => token_replace(check_markup($outro['value'], $outro['format']), array('user' => $user)),
  );
  return $output;
}

/**
 * Page callback for individual donation receipt
 */
function salesforce_donation_history_donation_receipt_page($user, $donation_id) {
  drupal_add_css(drupal_get_path('module', 'salesforce_donation_history') . '/salesforce_donation_history.css');
  $output = array();
  $intro = variable_get('salesforce_donation_history_donation_intro', NULL);
  $output['intro'] = array(
    '#type' => 'markup',
    '#markup' => token_replace(check_markup($intro['value'], $intro['format']), array('user' => $user)),
  );
  $user_sfid = salesforce_donation_history_user_account_id($user->uid);
  if ($user_sfid) {
    $results = salesforce_donation_history_get_opportunity($user_sfid, $donation_id);
  }
  if (count($results) > 0) {
    $output['table'] = salesforce_donation_history_create_table($results, 'show_on_receipt', TRUE, FALSE);
  }
  else {
    $output['no-donations'] = array(
      '#type' => 'markup',
      '#markup' => t('No donations found.'),
    );
  }
  $outro = variable_get('salesforce_donation_history_donation_outro', NULL);
  $output['footer'] = array(
    '#type' => 'markup',
    '#markup' => token_replace(check_markup($outro['value'], $outro['format']), array('user' => $user)),
  );
  return $output;
}
