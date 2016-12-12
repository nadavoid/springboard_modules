<?php

/**
 * @file
 * Provide a Multivariate Testing win condition: Highest sum of donations
 */

// Plugin details
$plugin = array(
  'title' => t('Highest Sum of Donation Amounts'),
  'description' => t('A donation form will be declared the winner if it meets all other win conditions, and has the highest sum of donation amounts among the donation forms included in the multivariate test.'),
  'config summary' => 'fundraiser_ab_donations_highest_gross_config_summary',
  'current value' => 'fundraiser_ab_donations_highest_gross_current_value',
  'check criteria' => 'fundraiser_ab_donations_highest_gross_check_win',
);

/**
 * Return a summary of the plugin config, for listing on the admin view of a
 * Multivariate Testing node
 */
function fundraiser_ab_donations_highest_gross_config_summary($config) {
  return 'Yes';
}

/**
 * Returns the current value of this win condition.
 *
 * @param $webform_nid
 *   Note ID for the webform being tested.
 * @param $node
 *   The Node Object of the multivariate test node.
 */
function fundraiser_ab_donations_highest_gross_current_value($webform_nid, $webform_details, $node) {
  // Get total, currency-agnostic donation amount for the current webform:
  $target_sum_donation = _fundraiser_ab_donations_highest_gross_get_sum_donation($webform_nid);

  foreach ($node->included_webforms as $nid => $details) {
    $this_sum_donation = _fundraiser_ab_donations_highest_gross_get_sum_donation($nid);
    if ($this_sum_donation > $target_sum_donation) {
      return 0;
    }
  }
  return 1;
}

/**
 * Get the average donation amount in US for a given webform:
 */
function _fundraiser_ab_donations_highest_gross_get_sum_donation($webform_nid) {
  $amount_query = db_query("SELECT amount, currency FROM {fundraiser_donation} WHERE nid = :nid",
    array(':nid' => $webform_nid));
  $sum = 0;
  foreach ($amount_query as $result) {
    $currency_amount = $result->amount;
    $currency_code = $result->currency;
    // Convert non-US currencies to USD for comparison's sake:
    if ($currency_code != "USD") {
      $currency_amount = commerce_currency_convert($currency_amount, $currency_code, 'USD');
    }
    $sum += $currency_amount;
  }
  return $sum;
}




/**
 * Check if the given Multivariate Test node has a winner
 * Checks the total donation amount for every donation form included in the
 * Multivariate Testing test, and checks if the one being checked has the highest
 *
 * @param $config
 *   Array of config options for this plugin for the Multivariate Testing test being
 *   checked for a win. In this case, the array will be empty.
 * @param $webform_details
 *   Array of info about the webform to check.
 * @param $test_node
 *   Multivariate Testing Test node
 */
function fundraiser_ab_donations_highest_gross_check_win($config, $webform_details, $test_node) {
  $highest_gross = 0;
  $highest_gross_nid = 0;
  // Loop over all the forms in this multivariate test
  foreach ($test_node->included_webforms as $nid => $details) {
    if (isset($details['fundraiser']) && is_array($details['fundraiser'])) {
      // If this form has earned more than the last highest, save it
      if ($details['fundraiser']['total_donations'] > $highest_gross) {
        $highest_gross = $details['fundraiser']['total_donations'];
        $highest_gross_nid = $nid;
      }
    }
  }

  // If the webform in question has the highest average, return TRUE
  return ($highest_gross_nid == $webform_details['webform_nid']);
}
