<?php
/**
 * @file
 * Fundraiser-specific hooks for Fundraiser Commerce Cardonfile.
 */

/**
 * Implements hook_fundraiser_commerce_update_donation_result_data_alter().
 *
 * Sets the donation's card_id value when the donation is updated.
 */
function fundraiser_commerce_cardonfile_fundraiser_commerce_update_donation_result_data_alter(&$donation) {
  $card = _fundraiser_commerce_donation_cardonfile_card($donation);
  if (!empty($card)) {
    $donation->data['cardonfile'] = $card->card_id;
  }
}

/**
 * Implements hook_fundraiser_donation_post_update().
 *
 * This function will update the 'field_data_commerce_cardonfile_profile'
 * field's value to the correct billing profile ID when a donation is created
 * or updated.
 *
 * @todo This can be removed once the commerce_cardonfile module supports
 * billing addresses for individual cards.
 */
function fundraiser_commerce_cardonfile_fundraiser_donation_post_update($donation) {
  $updated_donation = &drupal_static(__FUNCTION__, FALSE);
  if (!variable_get('fundraiser_commerce_cardonfile_patch_cardonfile', TRUE) || empty($donation->data['cardonfile']) || empty($donation->did) || !empty($updated_donation) || !empty($donation->cancelled)) {
    return;
  }

  $card = commerce_cardonfile_load($donation->data['cardonfile']);
  if (!empty($card)) {
    $card_wrapper = entity_metadata_wrapper('commerce_cardonfile', $card);

    $order = commerce_order_load($donation->did);
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

    $card_wrapper->commerce_cardonfile_profile = $order_wrapper->commerce_customer_billing->value();
    $card_wrapper->save();
  }

  $updated_donation = TRUE;
}

/**
 * Implements hook_fundraiser_commerce_donation_update_card_update_alter().
 *
 * Sets the donation's card_id value if $card_data is non-empty.
 */
function fundraiser_commerce_cardonfile_fundraiser_commerce_donation_update_card_update_alter(&$donation, $card_data) {
  if (!empty($card_data)) {
    $donation->data['cardonfile'] = $card_data->card_id;
    $donation->donation['cardonfile'] = $card_data->card_id;
  }
}

/**
 * Implements hook_fundraiser_commerce_sustainer_series_donation_update_alter().
 *
 * Sets the donation in a sustainer series card_id to its masters card_id value.
 */
function fundraiser_commerce_cardonfile_fundraiser_commerce_sustainer_series_donation_update_alter($master, &$donation) {
  if (!empty($master->data['cardonfile'])) {
    $donation->data['cardonfile'] = $master->data['cardonfile'];
    $donation->donation['cardonfile'] = $master->data['cardonfile'];
  }
}

/**
 * Implements hook_fundraiser_commerce_update_billing_profile_id_alter().
 *
 * Tell fundraiser_commerce what profile_id to use.
 */
function fundraiser_commerce_cardonfile_fundraiser_commerce_update_billing_profile_id_alter($donation, &$profile_id) {
  if (!empty($donation->data['cardonfile'])) {
    $profile_id = db_query('SELECT commerce_cardonfile_profile_profile_id FROM {field_data_commerce_cardonfile_profile} WHERE entity_id = :card_id', array(
      ':card_id' => $donation->data['cardonfile'],
    ))->fetchField();
    if (empty($profile_id)) {
      $profile_id = -1;
    }
  }
}
