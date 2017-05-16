<?php

/**
 * Page callback to handle the display for the form.
 * @param $nid
 * @return array|mixed
 */
function sba_message_action_download_page($node_id) {

  // CSS assets
  drupal_add_css(drupal_get_path('module', 'sba_message_action') . '/css/message-downloads.css');

  $form = drupal_get_form('sba_message_action_download_form', $node_id);

  $tabs = theme('messsage_deliverability_tabs', array(
    'nodeId' => $node_id
  ));

  $render_form = drupal_get_form('sba_message_action_download_form', $node_id);
  $download_report_form = drupal_render($render_form);

  $render_fail_messages_form = drupal_get_form('sba_message_action_fail_messages_form', $node_id);
  $fail_messages_form = drupal_render($render_fail_messages_form);

  // Load up the message action node.
  $node = node_load($node_id);

  // API Client instance
  $client = springboard_advocacy_api_call();
  $message_action_metrics = $client->invokeClientMethod('getMessageActionMetrics', $node->advocacy_id)->data;

  $page = theme('messsage_action_downloads', array(
    'form'                   => $download_report_form,
    'tabs'                   => $tabs,
    'fail_messages_form'     => $fail_messages_form,
    'message_action_metrics' => $message_action_metrics
  ));

  return $page;
}

/**
 * Generating the form rendering with form builder.
 * @param $form
 * @param $form_state
 * @param $node_id
 * @return mixed
 */
function sba_message_action_download_form($form, &$form_state, $node_id) {

  global $user;

  $form['description'] = array(
    '#type'  => 'item',
    '#title' => t('Each failed message can be downloaded only once. Please provide an email address to receive an export of all messages that failed to send since the previous download.'),
  );

  $form['name'] = array(
    '#type'          => 'textfield',
    '#name'          => 'csv_messages_download_email',
    '#title'         => t('Email'),
    '#default_value' => $user->mail,
    '#required'      => TRUE,
  );

  $form['node_id'] = array(
    '#type'          => 'value',
    '#name'          => 'node_id',
    '#title'         => t('Node Id'),
    '#default_value' => $node_id,
    '#required'      => TRUE,
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Get Download'),
  );

  return $form;
}

/**
 * Form submission handler for the sba message action download page.
 * Implements hook_submit()
 * @param $form
 * @param $form_state
 */
function sba_message_action_download_form_submit($form, &$form_state) {
  if (!form_get_errors()) {

    $message_nodeId = $form['node_id']['#value'];
    $email_address = $form_state['input']['csv_messages_download_email'];

    $form = node_load($message_nodeId);

    $message_advocacy_id = $form->advocacy_id;

    // Make the API request to queue up the CSV download
    $api_client = springboard_advocacy_api_call();
    $api_request = $api_client->invokeClientMethod('getFailedMessagesDownload', $message_advocacy_id, $email_address);

    if (isset($api_request->data) && $api_request->data == 'Download queued.') {
      drupal_set_message('Thanks! Your request has been submitted. You will receive an email shortly with a link to download any new failed messages.');
    }
    else {
      drupal_set_message('Sorry, your request did not go through. Please try again later.', 'error');
    }
  }
}

/**
 * Method to validate the form submission of the sba message action download page.
 * Implements hook_validate()
 * @param $form
 * @param $form_state
 */
function sba_message_action_download_form_validate($form, &$form_state) {

  if (!springboard_validate_email($form_state['input']['csv_messages_download_email'])) {
    form_set_error('csv_messages_download_email', t('Please enter a valid email address.'));
  }
}

/**
 * Generating the form rendering with form builder.
 * @param $form
 * @param $form_state
 * @param $node_id
 * @return mixed
 */
function sba_message_action_fail_messages_form($form, &$form_state, $node_id) {

  $form['node_id'] = array(
    '#type'          => 'value',
    '#name'          => 'node_id',
    '#title'         => t('Node Id'),
    '#default_value' => $node_id,
    '#required'      => TRUE,
  );

  $form['description'] = array(
    '#type'  => 'item',
    '#title' => t('Messages in "Hold" status can be updated to "Failed" status and downloaded above. We are still working to deliver "Hold" status messages, so we recommend you use this option only in time-sensitive campaigns where you wish to complete message delivery manually within a specific timeframe.'),
  );

  $form['caution'] = array(
    '#type'  => 'item',
    '#title' => t('Update to Failed Status (use with caution - this is not reversible)'),
  );

  $form['select'] = array(
    '#type'          => 'select',
    '#name'          => 'confirmed',
    '#options'       => array(
      0 => t('No'),
      1 => t('Yes'),
    ),
    '#default_value' => 0,
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Update Messages'),
  );

  return $form;
}

/**
 * Method to validate the form submission of the sba message action fail hold messages form.
 * Implements hook_validate()
 * @param $form
 * @param $form_state
 */
function sba_message_action_fail_messages_form_validate($form, &$form_state) {

  if ($form_state['input']['confirmed'] == 0) {
    form_set_error('confirmation_is_required', t('Please confirm you want to set all "Hold" status messages to "Failed" status.'));
  }
}

/**
 * Form submission handler for the sba message action fail hold messages form.
 * Implements hook_submit()
 * @param $form
 * @param $form_state
 */
function sba_message_action_fail_messages_form_submit($form, &$form_state) {

  if (!form_get_errors()) {

    $message_node_id = $form['node_id']['#value'];
    $form = node_load($message_node_id);

    $message_advocacy_id = $form->advocacy_id;

    // Make the API request to fail all messages in hold status
    $api_client = springboard_advocacy_api_call();
    $api_request = $api_client->invokeClientMethod('failHeldMessages', $message_advocacy_id);

    if (isset($api_request->data->message) && preg_match('/queue items are being failed./', $api_request->data->message)) {
      drupal_set_message('Success! ' . $api_request->data->items_updated . ' "Hold" status message(s) have been to set to "Failed" status.');
    }
    else {
      drupal_set_message('Sorry, your request did not go through. Please try again later.', 'error');
    }

  }
}