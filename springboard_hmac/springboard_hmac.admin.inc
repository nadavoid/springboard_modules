<?php

/**
 * @file
 * Admin functions for Springboard HMAC.
 */

/**
 * Page callback for the admin form.
 *
 * @return array $build
 *   The render array.
 */
function springboard_hmac_settings_page() {
  $build = array();
  $build['content'] = array(
    'page_description' => array(
      '#id' => 'springboard-advocacy-settings-page',
      '#type' => 'markup',
      '#markup' => '<p>' . t('Configure settings for Springboard HMAC.') . '</p>',
      '#prefix' => '<div class="springboard-hmac-settings-page">',
      '#suffix' => '</div>',
    ),
    'springboard_hmac_settings_form' => drupal_get_form('springboard_hmac_settings_form'),
  );
  return $build;
}

/**
 * Configuration settings form for Springboard advocacy.
 */
function springboard_hmac_settings_form() {

  $key = springboard_hmac_get_key();
  if (empty($key)) {
    drupal_set_message(t('Secure Prepopulate key and initialization vector are not set. Springboard HMAC will not function.'), 'error');
  }

  $form = array();

  $form['hmac'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['hmac']['springboard_hmac_error_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Page title'),
    '#default_value' => variable_get('springboard_hmac_error_title', ''),
  );

  $format = NULL;
  $message = variable_get('springboard_hmac_error_message', array('value' => 'An error occurred and we could not complete the operation.', 'format' => NULL));
  if (is_array($message)) {
    $text = $message['value'];
    $format = $message['format'];
  }
  else {
    $text = $message;
  }

  $form['hmac']['springboard_hmac_error_message'] = array(
    '#type' => 'text_format',
    '#format' => $format,
    '#title' => t('Error message'),
    '#default_value' => $text,
    '#required' => TRUE,
  );

  // Generator for test HMACs
  $form['hmac_generator'] = array(
    '#type' => 'fieldset',
    '#title' => t("Generate Test HMAC"),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['hmac_generator']['uid'] = array(
    '#type' => 'textfield',
    '#title' => t("UID"),
    '#default_value' => $GLOBALS['user']->uid,
  );
  $form['hmac_generator']['timestamp'] = array(
    '#type' => 'textfield',
    '#title' => t("Expiry timestamp"),
    '#default_value' => time() + 60 * 60 * 24,
  );
  $form['hmac_generator']['action_code'] = array(
    '#type' => 'textfield',
    '#title' => t("Action code"),
  );
  $form['hmac_generator']['params'] = array(
    '#type' => 'textfield',
    '#title' => t("Additional parameters"),
    '#description' => t("String with all additional parameters, delimited by periods."),
  );
  $form['hmac_generator']['op'] = array(
    '#type' => 'submit',
    '#value' => t("Generate"),
  );

  // Parser-verifier for testing.
  $form['hmac_parser'] = array(
    '#type' => 'fieldset',
    '#title' => t("Parse HMAC"),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['hmac_parser']['hmac_to_parse'] = array(
    '#type' => 'textarea',
    '#title' => t("Complete HMAC to parse"),
    '#description' => t("Slash-delimited payload and token."),
  );
  $form['hmac_parser']['op'] = array(
    '#type' => 'submit',
    '#value' => t("Parse"),
  );

  $form['#submit'][] = 'springboard_hmac_settings_form_submit';

  return system_settings_form($form);
}

function springboard_hmac_settings_form_submit($form, $form_state) {
  if ($form_state['clicked_button']['#value'] == t("Generate")) {
    if (empty($form_state['values']['uid'])) {
      form_set_error('uid', "UID required to generate HMAC.");
    }
    else if (empty($form_state['values']['timestamp'])) {
      form_set_error('timestamp', "Expiry timestamp required to generate HMAC.");
    }
    else if (empty($form_state['values']['action_code'])) {
      form_set_error('action_code', "Action code required to generate HMAC.");
    }
    else {
      $values = array(
        $form_state['values']['uid'],
        $form_state['values']['timestamp'],
        $form_state['values']['action_code'],
      );
      if (!empty($form_state['values']['params'])) {
        $values = array_merge($values, explode('.', $form_state['values']['params']));
      }
      $payload = implode('.', $values);
      drupal_set_message(t("Generated payload: %payload", array('%payload' => $payload)));
      $payload = base64_encode($payload);
      $key = springboard_hmac_get_key();
      $hmac = $payload . '/' . springboard_hmac_create_hmac($values, $key);
      drupal_set_message(t("Generated HMAC: %hmac", array('%hmac' => $hmac)));
    }
  }
  else if ($form_state['clicked_button']['#value'] == t("Parse")) {
    if (empty($form_state['values']['hmac_to_parse']) || strpos('/', $form_state['values']['hmac_to_parse'] === FALSE)) {
      form_set_error('hmac_to_parse', "No HMAC given to parse!");
    }
    else {
      list($payload, $hmac) = explode('/', $form_state['values']['hmac_to_parse']);
      $parsed_payload = springboard_hmac_parse_payload($payload);
      $error = springboard_hmac_verify_payload($parsed_payload);
      if (!empty($failure_reason)) {
        drupal_set_message(t("HMAC failed: %error", array('%error' => $error)), 'error');
      }
      else {
        $key = springboard_hmac_get_key();
        $verification_hmac = springboard_hmac_get_verification_hmac($parsed_payload, $key);

        // If the orginal token matches the one we just generated the data is
        // unaltered and we can continue.
        if ($hmac != $verification_hmac) {
          $reason = t('Authentication failed. Tokens do not match.');
          drupal_set_message(t("HMAC failed: %error", array('%error' => "Authentication failed. Tokens do not match.")), 'error');
        }
        else {
          drupal_set_message(t("HMAC verified with parsed payload %parsed_payload", array('%parsed_payload' => implode('.', $parsed_payload))));
        }
      }
    }
  }
}