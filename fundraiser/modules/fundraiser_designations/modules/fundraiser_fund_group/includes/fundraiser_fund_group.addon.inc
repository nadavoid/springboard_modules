<?php

/**
 * Create the add-on donation form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 * @param object $node
 *   A node this add-on belongs to.
 *
 * @return array $form
 *   The add-on form.
 */
function fundraiser_fund_groups_addon_form($form, $form_state, $node) {

  $data = !empty($node->fd_addon) ? $node->fd_addon : array();

  $addon_default = '';
  if (!empty($data['pid'])) {
    $addon = commerce_product_load($data['pid']);
    $addon_default = $addon->title . ' [' . $addon->product_id . ']';
  }

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $form['addon_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add-on Donation'),
    '#weight' => -4.96,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('An add-on donation is a one-time line item that will appear as an upsell opportunity on the form.'),
  );

  $form['addon_settings']['statuses'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable add-on donation'),
    '#weight' => -10,
    '#default_value' => isset($data['status']) ? $data['status'] : 0,
  );

  $form['addon_settings']['addon'] = array(
    '#type' => 'textfield',
    '#title' => t('Choose a fund designation'),
    '#autocomplete_path' => 'fund_groups/autocomplete_callback',
    '#weight' => -10,
    '#required' => TRUE,
    '#default_value' => $addon_default,
    '#states' => array(
      'visible' => array(
        ':input[name="statuses"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['addon_settings']['cart_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Cart display text'),
    '#weight' => -9,
    '#default_value' => !empty($data['cart_text']) ? $data['cart_text'] : 'One-time add-on donation',
    '#description' => t('The text that will appear in the shopping cart if a donor makes an add-on donation.'),
    '#required' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="statuses"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['addon_settings']['amount_wrapper'] = array(
    '#type' => 'fieldset',
    '#title' => t('Ask amounts'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="clear-block" id="ask-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="statuses"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['addon_settings']['amount_wrapper']['donation_amounts'] = array(
    '#prefix' => '<div id="ask-amounts">',
    '#suffix' => '</div>',
    '#theme' => 'fundraiser_ask_amounts',
    '#tree' => TRUE,
    '#required' => TRUE,

  );

  // AJAXy bits.
  $donation_amounts = array();
  // The last submission takes precedence.
  if (isset($form_state['values']['amount_wrapper']['donation_amounts'])) {
    foreach ($form_state['values']['amount_wrapper']['donation_amounts'] as $index => $values) {
      $donation_amounts[$values['weight']] = array($values['amount'], $values['label']);
    }
    ksort($donation_amounts);
  }
  else {
    $price_set = [];
    if (!empty($data['price_set'])) {
      $amounts = unserialize($data['price_set']);
      foreach ($amounts as $amount) {
        $price_set[] = array($amount['amount'], $amount['label']);
      }
    }
    $donation_amounts = $price_set;

    // If not there, then grab some defaults.
    if (count($donation_amounts) == 0) {
      $default_amounts = '25|$25' . "\n" . '50|$50' . "\n" . '75|$75';
      $default_amounts = explode("\n", $default_amounts);
      $default_amounts = array_unique($default_amounts);
      $exploded_amounts = array();
      foreach ($default_amounts as $default_amount) {
        $these_amounts = explode('|', $default_amount);
        if (count($these_amounts) == 1) {
          $these_amounts[1] = '$' . $these_amounts[0];
        }
        $exploded_amounts[] = $these_amounts;
      }
      $donation_amounts = $exploded_amounts;
    }
  }
  // Default provide one spare blank for users to fill in.
  // If the button clicked was to add another,
  // this line will add an extra field as expected on reload.
  $donation_amounts[] = array('', '');
  // Generate a form set for each existing amount.
  // (To be themed into table format).
  $index = 0;
  foreach ($donation_amounts as $weight => $donation_amount) {
    // Filter out other|Other combinations.
    // We don't want to display those in this form.
    // And if we did, the value would fail validation (not a number).
    // There is probably a better way to track this, but for now this'll do.
    if (count($donation_amount) == 2 && $donation_amount[0] != 'other') {
      $form['addon_settings']['amount_wrapper']['donation_amounts'][$index] = _fundraiser_ask_form($donation_amount, $index, $node, $weight);
    }
    ++$index;
  }

  // AJAX fix: reorder donation amounts in $form_state['input'] by weight.
  // Corrects the display order of donation amounts when an ajax event is
  // triggered.
  if (isset($form_state['input']['amount_wrapper']['donation_amounts'])) {
    $old_donation_amounts = $form_state['input']['amount_wrapper']['donation_amounts'];
    unset($form_state['input']['amount_wrapper']['donation_amounts']);
    foreach ($old_donation_amounts as $donation_amount) {
      $new_donation_amounts[$donation_amount['weight']] = $donation_amount;
    }
    ksort($new_donation_amounts);
    foreach ($new_donation_amounts as $donation_amount) {
      $form_state['input']['amount_wrapper']['donation_amounts'][] = $donation_amount;
    }
  }

  $form['addon_settings']['amount_wrapper']['amount_more'] = array(
    '#type' => 'button',
    '#submit' => array(),
    '#value' => t('Add another'),
    '#description' => t('Click here to add more choices.'),
    '#ajax' => array(
      'callback' => '_fundraiser_fund_groups_settings_form_amounts',
      'wrapper' => 'ask-amounts',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#attached']['css'][] = drupal_get_path('module', 'fundraiser_fund_group') . '/css/fund-groups.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'fundraiser_designations') . '/js/price-sets.js';
  $form['#validate'] = array('fundraiser_fund_groups_addon_form_validate');
  $form['#submit'] = array('fundraiser_fund_groups_addon_form_submit');

  return $form;
}

/**
 * Ajax replacement.
 */
function _fundraiser_fund_groups_settings_form_amounts(&$form, &$form_state) {
  return $form['addon_settings']['amount_wrapper']['donation_amounts'];
}

/**
 * Autocomplete callback for the add-on fund selector.
 *
 * @param string $string
 *   The commerce product title.
 */
function fundraiser_fund_groups_autocomplete_callback($string = "") {
  $matches = array();
  if ($string) {
    $result = db_select('commerce_product')
      ->fields('commerce_product', array('title', 'product_id'))
      ->condition('title', '%' . db_like($string) . '%', 'LIKE')
      ->condition('type', 'fundraiser_designation', '=')
      ->condition('status', 1, '=')
      ->range(0, 10)
      ->execute();
    foreach ($result as $designation) {
      $matches[$designation->title . " [$designation->product_id]"] = check_plain($designation->title);
    }
  }
  drupal_json_output($matches);
}


/**
 * Implements hook_form_validate().
 */
function fundraiser_fund_groups_addon_form_validate($form, &$form_state) {
  foreach ($form_state['values']['amount_wrapper']['donation_amounts'] as $index => $amount_set) {
    if (!empty($amount_set['amount']) && empty($amount_set['label'])) {
      form_set_error('', t('The label cannot be empty when an amount is set.'));
    }
    if (empty($amount_set['amount']) && !empty($amount_set['label'])) {
      form_set_error('', t('The amount cannot be empty when a label is set.'));
    }
    if (!is_numeric($amount_set['amount']) && !empty($amount_set['amount'])) {
      form_set_error('', t('The amount must be a number.'));
    }
  }

  $title = $form_state['values']['addon'];
  $matches = array();

  // This preg_match() looks for the last pattern like [33334] and if found
  // extracts the numeric portion.
  $result = preg_match('/\[([0-9]+)\]$/', $title, $matches);
  if ($result > 0) {
    // If $result is nonzero, we found a match and can use it as the index into
    // $matches.
    $pid = $matches[$result];
    // Verify that it's a valid nid.
    $designation = commerce_product_load($pid);
    if (empty($designation)) {
      form_set_error('addon', t('Sorry, no designation with id %nid can be found', array('%nid' => $pid)));
      return;
    }
  }
  else {
    if (empty($designation)) {
      form_set_error('addon', t('Sorry, you must choose a fund.'));
      return;
    }
  }
  if (!empty($pid)) {
    $form_state['values']['addon'] = $pid;
  }
}

/**
 * Impelments hook_form_submit().
 */
function fundraiser_fund_groups_addon_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $sets = [];

  db_delete('fundraiser_fund_group_addon')
    ->condition('nid', $values['nid'], '=')
    ->execute();

  foreach ($values['amount_wrapper']['donation_amounts'] as $amount) {
    if (empty($amount['remove']) && !empty($amount['amount'])) {
      $sets[] = $amount;
    }
  }
  $data = array(
    'nid' => $values['nid'],
    'pid' => $values['addon'],
    'status' => (int) $values['statuses'],
    'cart_text' => $values['cart_text'],
    'price_set' => $sets,
  );
  drupal_write_record('fundraiser_fund_group_addon', $data);

}
