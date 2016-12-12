<?php

/**
 * @file
 * Admin file for the Springboard Measurement Protocol module.
 */

/**
 * Page callback for admin settings.
 */
function springboard_measurement_protocol_admin() {
  $form = array();
  $form['general_settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('General Settings'),
  );
  $form['general_settings']['springboard_measurement_protocol_ga_account'] = array(
    '#title' => t('Web Property ID'),
    '#type' => 'textfield',
    '#default_value' => variable_get('springboard_measurement_protocol_ga_account', 'UA-'),
    '#size' => 15,
    '#maxlength' => 20,
    '#required' => TRUE,
    '#description' => t('This ID is unique to each site you want to track separately, and is in the form of UA-xxxxxxx-yy. To get a Web Property ID, <a href="@analytics">register your site with Google Analytics</a>, or if you already have registered your site, go to your Google Analytics Settings page to see the ID next to every site profile. <a href="@webpropertyid">Find more information in the documentation</a>.', array(
      '@analytics' => 'http://www.google.com/analytics/',
      '@webpropertyid' => url('https://developers.google.com/analytics/resources/concepts/gaConceptsAccounts', array('fragment' => 'webProperty')),
    )),
  );
  $form['general_settings']['springboard_measurement_protocol_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log details in watchdog about requests made to the Measurement Protocol service'),
    '#description' => t('Enable if you are debugging.'),
    '#default_value' => variable_get('springboard_measurement_protocol_debug', FALSE),
  );
  $form['paypal_events'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('PayPal Events'),
  );
  $form['paypal_events']['springboard_measurement_protocol_paypal_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send events for PayPal transactions'),
    '#description' => t("Send analytics to Google's Measurement Protocol service when a PayPal transaction occurs."),
    '#default_value' => variable_get('springboard_measurement_protocol_paypal_enable', FALSE),
  );

  return system_settings_form($form);
}
