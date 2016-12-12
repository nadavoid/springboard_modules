<?php

/**
 * @file
 * Administration page callbacks for the salesforce nonprofit starter pack module.
 */

/**
 * Page callback for the admin form.
 */
function salesforce_npsp_admin_page() {
  $build = array();
  $build['content'] = array(
    'page_description' => array(
      '#id' => 'salesforce-npsp-admin-helptext-page',
      '#type' => 'markup',
      '#markup' => '<p>' . t('Configure settings for Salesforce Nonprofit Starter Pack.') . '</p>',
      '#prefix' => '<div class="npsp-admin-helptext">',
      '#suffix' => '</div>',
    ),
    'salesforce_npsp_admin_form' => drupal_get_form('salesforce_npsp_admin_form'),
  );
  return $build;
}

/**
 * Configuration form for npsp admin options.
 */
function salesforce_npsp_admin_form($form, &$form_state) {
  $form = array();

  // Variable salesforce_npsp_lastname.
  $form['lastname'] = array(
    '#id' => 'salesforce-npsp-lastname',
    '#title' => t('Default Sync Last Name'),
    '#description' => t('When an item fails sync because it lacks a lastname, this field indicates what will be set for the lastname.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('salesforce_npsp_lastname', SALESFORCE_NPSP_DEFAULT_LASTNAME),
    '#required' => TRUE,
  );

  $form['user_profile_info'] = array(
    '#type' => 'checkbox',
    '#title' => t("Show Salesforce information on the user's profile page?"),
    '#default_value' => variable_get('salesforce_npsp_user_profile', TRUE),
  );

  // Save.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
  );

  return $form;
}

/**
 * Validation callback for configuration form.
 */
function salesforce_npsp_admin_form_validate($form, &$form_state) {

}

/**
 * Submit callback for the configuration form.
 */
function salesforce_npsp_admin_form_submit($form, &$form_state) {
  // Set default last name.
  variable_set('salesforce_npsp_lastname', $form_state['input']['lastname']);
  // Set default value for displaying Salesforce info on user profile.
  variable_set('salesforce_npsp_user_profile', !empty($form_state['input']['user_profile_info']));
}
