<?php

/**
 * @file
 * Forms for editing donations.
 */

/**
 * Form callback: edit a donation.
 *
 * We are only allowing access to edit entity fields. Values in the fundraiser_donation table are left alone.
 *
 * @param $fundraiser_donation
 *   The $fundraiser_donation entity object to edit through the form.
 */
function fundraiser_entity_donation_entity_form($form, &$form_state, $fundraiser_donation) {
  // Ensure this include file is loaded when the form is rebuilt from the cache.
  $form_state['build_info']['files']['form'] = drupal_get_path('module', 'fundraiser_entity') . '/includes/fundraiser_entity.admin.inc';

  // Add the field related form elements.
  $form_state['fundraiser_donation'] = $fundraiser_donation;
  field_attach_form('fundraiser_donation', $fundraiser_donation, $form, $form_state);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 40,
    '#value' => 'Submit'
  );

  return $form;
}

/**
 * Validate callback for fundraiser_entity_donation_entity_form().
 */
function fundraiser_entity_donation_entity_form_validate($form, $form_state) {
  $fundraiser_donation = $form_state['fundraiser_donation'];

  // Notify field widgets to validate their data.
  field_attach_form_validate('fundraiser_donation', $fundraiser_donation, $form, $form_state);
}

/**
 * Submit callback for fundraiser_entity_donation_entity_form().
 */
function fundraiser_entity_donation_entity_form_submit($form, &$form_state) {
  $fundraiser_donation = $form_state['fundraiser_donation'];

  // Notify field widgets.
  field_attach_submit('fundraiser_donation', $fundraiser_donation, $form, $form_state);

  // Save the donation.
  entity_save('fundraiser_donation', $fundraiser_donation);
}
