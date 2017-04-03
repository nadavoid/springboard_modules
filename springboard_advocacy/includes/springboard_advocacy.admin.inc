<?php

/**
 * @file
 * Admin functions for Springboard advocacy.
 */

/**
 * Page callback for the admin form.
 */

function springboard_advocacy_settings_page() {
  $build = array();
  $build['content'] = array(
    'page_description' => array(
      '#id' => 'springboard-advocacy-settings-page',
      '#type' => 'markup',
      '#markup' => '<p>' . t('Configure settings for Springboard advocacy.') .
        '</p><p>' .
        t('Saving this form will also force a rebuild of the advocacy subscription settings and renew the Oauth Access Token.') .
        '</p>',
      '#prefix' => '<div class="springboard-advocacy-settings-page">',
      '#suffix' => '</div>',
    ),
    'springboard_advocacy_settings_form' => drupal_get_form('springboard_advocacy_settings_form'),
  );
  return $build;
}

/**
 * Configuration settings form for Springboard advocacy.
 */
function springboard_advocacy_settings_form() {
  $form = array();


  $form['transaction'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advocacy Server Configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['transaction']['springboard_advocacy_debug'] = array(
    '#title' => t('Debug mode'),
    '#description' => t('Log extensive debug info to watchdog. Not recommended for production sites.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('springboard_advocacy_debug', ''),
  );

  $form['transaction']['springboard_advocacy_test_email'] = array(
    '#title' => t('Default test mode email address'),
    '#description' => t('When a Message Action node is in test mode, messages are sent to a test mode email address instead of the designated targets. Please enter a default email to use for test mode. This can be overridden on a per node basis.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('springboard_advocacy_test_email', ''),
    '#required' => TRUE,
    '#element_validate' => array('springboard_advocacy_email_validate'),
  );

  $form['transaction']['springboard_advocacy_server_url'] = array(
    '#title' => t('Springboard advocacy server url'),
    '#description' => t('The url of the Springboard advocacy server. Use "https" for the gospringboard.io servers.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('springboard_advocacy_server_url', ''),
    '#required' => TRUE,
    '#element_validate' => array('springboard_advocacy_url_validate'),

  );

  $form['transaction']['springboard_advocacy_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Transaction server Oauth Client ID.'),
    '#description' => t('Client ID for legislator lookup'),
    '#default_value' => variable_get('springboard_advocacy_client_id', ''),
    '#required' => TRUE,
  );

  $form['transaction']['springboard_advocacy_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Transaction server Oauth Client Secret.'),
    '#description' => t('Client Secret for legislator lookup'),
    '#default_value' => variable_get('springboard_advocacy_client_secret', ''),
    '#required' => TRUE,
  );

  $form['geo'] = array(
    '#type' => 'container',
  );

  $form['geo']['springboard_advocacy_geocoder'] = array(
    '#type' => 'hidden',
    '#value' => 'smartystreets_rest',
  );

  $form['geo']['smarty'] = array(
    '#type' => 'fieldset',
    '#title' => t('Smarty Streets configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['geo']['smarty']['springboard_advocacy_smarty_authid'] = array(
    '#type' => 'textfield',
    '#title' => t('Smarty Streets Auth ID'),
    '#description' => t('Required for the REST API'),
    '#default_value' => variable_get('springboard_advocacy_smarty_authid', ''),
    '#required' => TRUE,
  );

  $form['geo']['smarty']['springboard_advocacy_smarty_authtoken'] = array(
    '#type' => 'textfield',
    '#title' => t('Smarty Streets Auth Token'),
    '#description' => t('Required for the REST API'),
    '#default_value' => variable_get('springboard_advocacy_smarty_authtoken', ''),
    '#required' => TRUE,
  );

  $form['geo']['smarty']['springboard_advocacy_smarty_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log Smarty API calls to Watchdog'),
    '#description' => t('Log smarty responses watchdog. Not recommended for production sites.'),
    '#default_value' => variable_get('springboard_advocacy_smarty_debug', ''),
  );

  if(module_exists('sba_social_action')) {

    // Twitter Oauth credentials. These are different than the credentials in the
    // settings of the Drupal Twitter module, the difference allowing us to run
    // multiple apps for the same website, while continuing to use the helper functions
    // that twitter.module provides.
    $form['twitter'] = array(
      '#type' => 'fieldset',
      '#title' => t('Twitter Configuration'),
      '#description' => '',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['twitter']['social_actions_twitter_callback_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Social Action Twitter Callback URL'),
      '#default_value' => variable_get('social_actions_twitter_callback_url', 'social-action/oauth'),
      '#description' => t('This path is set in the Twitter app on twitter.com. Please set that to "%path", or whatever custom value you enter here.', array('%path' => 'social-action/oauth')),
    );

    $form['twitter']['social_action_twitter_consumer_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Social Action Twitter OAuth Consumer key'),
      '#default_value' => variable_get('social_action_twitter_consumer_key', NULL),
    );

    $form['twitter']['social_action_twitter_consumer_secret'] = array(
      '#type' => 'textfield',
      '#title' => t('Social Action Twitter OAuth Consumer secret'),
      '#default_value' => variable_get('social_action_twitter_consumer_secret', NULL),
    );

    $form['sba_social_action_cache_expire_period'] = array(
      '#type' => 'textfield',
      '#title' => t('Oauth Session Expiration Period'),
      '#default_value' => variable_get('sba_social_action_cache_expire_period', 1),
      '#description' => t('The time, in hours, in which an Oauth session should expire. 0.5 = 30 minutes.')
    );

    $form['sba_social_privacy'] = array(
      '#type' => 'textfield',
      '#title' => t('Privacy policy url'),
      '#default_value' => variable_get('sba_social_privacy', NULL),
      '#element_validate' => array('springboard_advocacy_url_validate'),
      '#description' => t('Twitter and other social platforms require that you display a privacy policy link to users before they sign into your app. A link to this url will appear below any social platform login buttons.')
      );
  }

  return system_settings_form($form);
}

function springboard_advocacy_url_validate($element, &$form_state) {

  if (!valid_url($form_state['values']['springboard_advocacy_server_url'], TRUE)) {
    form_error($element, t('You must enter a valid, fully-prefixed absolute url.'));
  }

  if (!empty($form_state['values']['sba_social_privacy']) && !valid_url($form_state['values']['sba_social_privacy'], TRUE)) {
    form_error($element, t('You must enter a valid, fully-prefixed absolute url.'));
  }

  $non_local = strpos($form_state['values']['springboard_advocacy_server_url'], 'gospringboard.io');
  $https = strpos($form_state['values']['springboard_advocacy_server_url'], 'https');

  if ($non_local !== FALSE && $https === FALSE) {
    form_error($element, t('You must use an "https" prefix when connecting to the gospringboard.io server.'));
  }
}

function springboard_advocacy_email_validate($element, &$form_state) {

  if (!valid_email_address($form_state['values']['springboard_advocacy_test_email'], TRUE)) {
    form_error($element, t('You must enter a valid email address'));
  }

}


/**
 * Message action settings page, menu callback.
 *
 * @return array
 */
function springboard_advocacy_confirmation_settings_page() {
  $build = array();
  $build['content'] = array(
    'page_description' => array(
      '#id' => 'message-action-settings-page',
      '#type' => 'markup',
      '#markup' => '<p>' . t('Node type settings for Springboard Advocacy.'),
      '#prefix' => '<div class="message-action-settings-page">',
      '#suffix' => '</div>',
    ),
    'springboard_advocacy_settings_form' => drupal_get_form('springboard_advocacy_confirmation_settings_form'),
  );
  return $build;
}

/**
 * Message action settings form
 * Provides a textarea for a global confirmation message
 * and available tokens.
 * @return mixed
 */
function springboard_advocacy_confirmation_settings_form() {

  $action_types = array_filter(variable_get('sba_action_types' , array()));
  if(!empty($action_types)) {
    $form['confirmation'] = array(
      '#type' => 'fieldset',
      '#title' => t('Confirmation failure message defaults'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => -10,
    );

    foreach ($action_types as $type) {
      //Create the confirmation form textarea
      $form['confirmation'][$type . '_confirm'] = array(
        '#type' => 'textarea',
        '#title' => t('Default @type Confirmation Failure Message', array('@type' => node_type_get_name($type))),
        '#default_value' => variable_get($type . '_confirm', ''),
        '#required' => TRUE,
      );
    }
  }
  else {
    $form['message']['#markup'] = t('No action modules are enabled to configure');
  }
  //$form['#submit'][] = 'springboard_advocacy_settings_form_submit';
  return system_settings_form($form);
}


/**
 * Submit function for message action settings.
 *
 * Updates all message action nodes which do not use a custom confirmation
 * with the global confirmation.
 * @param $form
 * @param $form_state
 */
//function springboard_advocacy_confirmation_settings_form_submit($form, $form_state) {
//  // Get all action node nids that do not have a custom confirmation record.
//  $result = db_query('SELECT n.nid, s.custom_confirmation FROM {node} n
//    LEFT JOIN {sba_message_action_confirm} s
//    ON s.nid = n.nid WHERE n.type = :type && (s.custom_confirmation = 0
//    OR s.nid IS NULL)', array(':type' => 'sba_message_action'));
//
//  // Update the results with the global confirmation.
//  foreach ($result as $node) {
//    $record = array(
//      'nid' => $node->nid,
//      'fail_message' => variable_get('sba_message_action_confirm', ''),
//    );
//    drupal_write_record('sba_message_action_confirm', $record, 'nid');
//  }
//}
