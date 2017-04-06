<?php
/**
 * @file
 * Mail forms and functions for fundraiser upsell.
 */

/**
 * Admin form for the thank you email functions.
 */
function fundraiser_upsell_admin_thank_you_settings() {
  // Get some default values.
  $form = array();
  $site_mail = variable_get('site_mail', '');
  $site_name = variable_get('site_name', '');
  $thank_you_content = variable_get('fundraiser_upsell_thank_you_content', '');
  $recipient_description = t("This is used to optionally send a test. The final email will be sent to the donation user. For more than one address, addresses should be separated by a comma. Example: test1@example.com,test2@example.com");
  // Add the thank you content fields.
  $form['fundraiser_upsell_thank_you_content'] = array(
    '#title' => t('Default content for Upsell Thank You'),
    '#type' => 'textarea',
    '#default_value' => $thank_you_content,
    '#weight' => -20,
  );

  if (!module_exists('springboard_token_set')) {
    $form['fundraiser_upsell_thank_you_content_tokens'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available thank you message tokens'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => -19,
    );
    $form['fundraiser_upsell_thank_you_content_tokens']['help'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array('all', 'donation'),
      '#global_types' => TRUE,
      '#click_insert' => TRUE,
    );
  }

  $form['email_title'] = array(
    '#type' => 'item',
    '#title' => t('E-mail settings'),
    '#weight' => -10,
  );
  // Pull in the required include file
  // to get access to email_wrappers helper functions.
  module_load_include('inc', 'email_wrappers', 'email_wrappers.api');
  module_load_include('module', 'email_wrappers');
  // Set up some initial parameters for our new mail configuration form.
  $params = array(
    'name' => 'fundraiser_upsell',
    'module' => 'fundraiser_upsell',
    'delta' => 'thank_you_settings',
  );
  // Generate the mail configuration form.
  $email_form = email_wrappers_mail_form(array(), $params);
  // Add the email form.
  $form['email'] = $email_form;
  $form['email']['#type'] = 'vertical_tabs';
  $form['email']['#weight'] = -9;
  // Modify the headers section.
  $form['email']['headers']['#title'] = "Mail headers";
  $form['email']['headers']['#weight'] = -5;
  if (empty($form['email']['headers']['from_name']['#default_value'])) {
    $form['email']['headers']['from_name']['#default_value'] = $site_name;
  }
  if (empty($form['email']['headers']['from_name']['#default_value'])) {
    $form['email']['headers']['from_name']['#default_value'] = $site_mail;
  }
  $form['email']['headers']['recipients']['#title'] = t('Test Email Recipient');
  $form['email']['headers']['recipients']['#description'] = $recipient_description;
  $form['email']['headers']['recipients']['#weight'] = 1;
  if (empty($form['email']['headers']['recipients']['#default_value'])) {
    $form['email']['headers']['recipients']['#default_value'] = $site_mail;
  }
  // Add an additional button we can use to send a test mail.
  $form['email']['headers']['fundraiser_upsell_send'] = array(
    '#type' => 'button',
    '#value' => 'Send test email',
    '#executes_submit_callback' => TRUE,
    '#submit' => array('fundraiser_upsell_send_test'),
    '#weight' => 2,
  );
  $form['email']['headers']['fundraiser_upsell_send_note'] = array(
    '#markup' => t('Test emails will not have replaced tokens.'),
    '#weight' => 3,
  );

  // Modify the message section.
  $form['email']['message']['#title'] = "Mail body";
  $form['email']['message']['#weight'] = -6;
  if (empty($form['email']['message']['subject']['#default_value'])) {
    $form['email']['message']['subject']['#default_value'] = "Thank you for becoming a sustaining partner.";
  }

  if (!module_exists('springboard_token_set')) {
    $form['email']['message']['tokens'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available email message tokens'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 19,
    );
    $form['email']['message']['tokens']['help'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array('all', 'donation'),
      '#global_types' => TRUE,
      '#click_insert' => TRUE,
    );
  }

  // Move the button and validation.
  $form['save'] = $form['email']['save'];
  $form['#validate'] = $form['email']['#validate'];
  $form['#submit'] = $form['email']['#submit'];
  $form['#submit'][] = 'fundraiser_upsell_admin_thank_you_submit';
  unset($form['email']['save']);
  unset($form['email']['#validate']);
  unset($form['email']['#submit']);

  return $form;
}

/**
 * Submit handler for non-email wrapper fields on the thank you settings form.
 */
function fundraiser_upsell_admin_thank_you_submit($form, $form_state) {
  variable_set('fundraiser_upsell_thank_you_content', $form_state['values']['fundraiser_upsell_thank_you_content']);
}

/**
 * Button submit callback, triggers mail send when clicked.
 */
function fundraiser_upsell_send_test($form, $form_state) {
  $donation = NULL;
  // Run it through the normal submit functions to save values.
  email_wrappers_mail_form_submit($form, $form_state);
  fundraiser_upsell_admin_thank_you_submit($form, $form_state);
  // Now, process and send the test email.
  module_load_include('inc', 'email_wrappers', 'email_wrappers.api');
  $params = array(
    'name' => 'email_admin_form',
    'module' => 'fundraiser_upsell',
    'delta' => 'thank_you_settings',
  );
  $mail = email_wrappers_mail_load($params);
  // Modify the messages to replace tokens.
  $mail->subject = token_replace($mail->subject, array(
      'global' => TRUE,
      'donation' => $donation,
      'domain' => NULL,
    ));
  $mail->html_message = token_replace($mail->html_message, array(
      'global' => TRUE,
      'donation' => $donation,
      'domain' => NULL,
    ));
  $mail->text_message = token_replace($mail->text_message, array(
      'global' => TRUE,
      'donation' => $donation,
      'domain' => NULL,
    ));
  email_wrappers_send_mail($mail, $params);
}

/**
 * Send the thank you email after the upsell.
 */
function fundraiser_upsell_send_thank_you($donation) {
  $to_email = $donation->donation['mail'];
  $to_name = $donation->donation['first_name'] . ' ' . $donation->donation['last_name'];
  module_load_include('inc', 'email_wrappers', 'email_wrappers.api');
  $params = array(
    'name' => 'fundraiser_upsell',
    'module' => 'fundraiser_upsell',
    'delta' => 'nid_' . $donation->nid,
  );
  // Load the email templates options.
  $mail = email_wrappers_mail_load($params);
  // Modify the "to:" information to match the donation.
  $mail->recipients = "$to_name <$to_email>";

  // Replace tokens in the subject.
  $mail->subject = token_replace(
    $mail->subject,
    array('donation' => $donation),
    array('clear => TRUE')
  );
  // Modify the messages to replace tokens.
  $mail->html_message = token_replace(
    $mail->html_message,
    array('donation' => $donation),
    array('clear' => TRUE)
  );
  $mail->text_message = token_replace(
    $mail->text_message,
    array('donation' => $donation),
    array('clear' => TRUE)
  );
  email_wrappers_send_mail($mail, $params);
}
