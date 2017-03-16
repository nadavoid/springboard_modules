<?php

/**
 * @file
 * Provide a Multivariate test win condition: Highest average of donations
 */

// Plugin details
$plugin = array(
  'title' => t('Highest Average of Donation Amounts'),
  'description' => t('A donation form will be declared the winner if it meets all other win conditions, and has the highest average donation amount among the donation forms included in the multivariate test.'),
  'config summary' => 'fundraiser_ab_donations_highest_avg_config_summary',
  'current value' => 'fundraiser_ab_donations_highest_avg_current_value',
  'check criteria' => 'fundraiser_ab_donations_highest_avg_check_win',
);



/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate test node
 */
function fundraiser_ab_donations_highest_avg_config_summary($config) {
  return 'Yes';
}

/**
 * Returns the current value of this win condition.
 *
 * @param $webform_nid
 *   Note ID for the webform being tested.
 */
function fundraiser_ab_donations_highest_avg_current_value($webform_nid, $webform_details, $node) {
  // Get average, currency-agnostic donation amount for the current webform:
  $target_average_donation = _fundraiser_ab_donations_highest_avg_get_average_donation($webform_nid);

  foreach ($node->included_webforms as $nid => $details) {
    $this_avg_donation = _fundraiser_ab_donations_highest_avg_get_average_donation($nid);
    if ($this_avg_donation > $target_average_donation || ($this_avg_donation === 0 && $target_average_donation === 0)) {
      return 0;
    }
  }
  return 1;
}

/**
 * Get the average donation amount in US for a given webform:
 */
function _fundraiser_ab_donations_highest_avg_get_average_donation($webform_nid) {
  $amount_query = db_query("SELECT amount, currency FROM {fundraiser_donation} WHERE nid = :nid",
    array(':nid' => $webform_nid));
  $sum = 0; $count = 0;
  foreach ($amount_query as $result) {
    $currency_amount = $result->amount;
    $currency_code = $result->currency;
    // Convert non-US currencies to USD for comparison's sake:
    if ($currency_code != "USD") {
      $currency_amount = commerce_currency_convert($currency_amount, $currency_code, 'USD');
    }
    $sum += $currency_amount;
    $count++;
  }
  if ($count == 0) {
    return 0;
  }
  return $sum / $count;
}

/**
 * Check if the given multivariate test node has a winner
 * Checks the average donation amount for every donation form included in the
 * multivariate test, and checks if the one being checked has the highest average
 *
 * @param $config
 *   Array of config options for this plugin for the multivariate test being
 *   checked for a win. In this case, the array will be empty.
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate test node
 */
function fundraiser_ab_donations_highest_avg_check_win($config, $webform_details, $test_node) {
  $highest_avg = 0;
  $highest_avg_nid = 0;
  // Loop over all the forms in this nultivariate test
  foreach ($test_node->included_webforms as $nid => $details) {
    if (isset($details['fundraiser']) && is_array($details['fundraiser']) && $details['fundraiser']['donation_count'] > 0) {
      // Determine the average donation amount for this donation form
      $avg = $details['fundraiser']['total_donations'] / $details['fundraiser']['donation_count'];

      // If this average is higher than the last highest, save it
      if ($avg > $highest_avg) {
        $highest_avg = $avg;
        $highest_avg_nid = $nid;
      }
    }
  }

  // If the webform in question has the highest average, return TRUE
  return ($highest_avg_nid == $webform_details['webform_nid']);
}
