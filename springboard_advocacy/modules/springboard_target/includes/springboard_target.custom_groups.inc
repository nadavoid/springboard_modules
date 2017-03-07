<?php

/**
 * @file
 * Form definition for Group creation and editing.
 */

/**
 * Serves a dual function as group add and edit form.
 *
 * @param null $group_id
 * @return array
 */
function springboard_target_group_form($group_id = NULL) {
  $form = array();

  // Attach our target form js and css.
  $form['#attached']['css'][] = drupal_get_path('module', 'springboard_target') . '/css/sba_target.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'springboard_target') . '/js/sba_target.js';

  // Display a warning if the API is unconfigured and return an empty form.
  $unconfigured = springboard_advocacy_api_unconfigured('springboard');
  if(!empty($unconfigured)) {
    $items = implode(', ', $unconfigured);
    $message = t('Please <a href="!url">configure</a> the Springboard API settings. There are empty values in :items. Configuration is required prior to group creation.', array('!url' => url('admin/config/services/advocacy'), ':items' => $items));
    $form = array();
    $form['not_found']['#markup'] = $message;
    return $form;
  }

  $form['#attributes']['class'][] = 'group-edit-form';

  // Load the target if Group ID is present.
  if (isset($group_id) && is_numeric($group_id)) {
    $api_call = springboard_advocacy_api_call();
    $response = $api_call->invokeClientMethod('getTargetGroup', $group_id);

    if (isset($response->error)) {
      $form = array();
      $form['not_found']['#markup'] = $response->error->message;
      return $form;
    }
    $group = $response->data;
  }
  else {
    $group = NULL;
  }

  $form['group_group_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Group Name'),
    '#default_value' => isset($group->name) ? $group->name : '',
    '#description' => t('Provide a descriptive name for your group.'),
    '#size' => 40,
    '#maxlength' => 128,
    '#required' => TRUE,
    '#attributes' => array('class' => array('group-add', 'group-name')),
  );

  $form['data']['group_id'] = array(
    '#type' => 'hidden',
    '#default_value' => $group_id,
  );

  if(module_exists('sba_target_search')) {
    $form = sba_target_search_add_target_search_form($form, $form_state, $group);
  }

  return $form;
}

/**
 * Validation handler for Group create/edit forms
 * Submits the group via the Advocacy API
 */

function springboard_target_group_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $is_new = TRUE;
  $group = array(
    'recipients' => '',
    'name' => '',
    'is_template' => 1,
  );
  $errors = FALSE;
  if(isset($values['group_group_name']) && $values['group_group_name'] != '') {
    $group['name'] = $values['group_group_name'];
  }
  else {
    form_set_error('group_group_name]', 'Please enter a valid name for your group.');
    $errors = TRUE;
  }
  if(isset($values['data']['group_id']) && $values['data']['group_id'] != '') {
    $group['group_id'] = $values['data']['group_id'];
    $is_new = FALSE;
  }
  if(isset($values['data']['recipients']) && !$errors) {
    $group['recipients'] = $values['data']['recipients'];
    module_load_include('inc', 'springboard_target', 'includes/springboard_target.api_calls');
    $group_id = springboard_target_api_save_target_group($group, $is_new);
    if ($group_id) {
      drupal_set_message('The group was successfully saved to the advocacy server.', 'status');
      $form_state['redirect'] = 'admin/springboard/advocacy/targets/custom-groups';
    }
    else {
      form_set_error('data][recipients', t('There was a problem saving the group @name to the advocacy server.', array('@name' => $values['group_group_name'])));
    }
  }
}

/**
 * Delete a custom target.
 *
 * @param $form
 * @param $form_state
 * @param $group_id
 * @return mixed
 *
 */
function springboard_target_group_delete($group_id) {
  // Load group to determine if it is eligible for deletion.
  $api_call = springboard_advocacy_api_call();
  $response = $api_call->invokeClientMethod('getTargetGroup', $group_id);

  // If the group has parents (is in use by another group or message) it cannot be deleted.
  if(isset($response->data) && (!empty($response->data->parents))) {
    $output = "<p>" . t("The group @name is in use and cannot be deleted. ", array("@name" => $response->data->name)) . l('Return to the Groups list.', 'admin/springboard/advocacy/targets/custom-groups') . "</p>";
  }
  else if (isset($response->data)) {
    $output = drupal_get_form('springboard_target_group_delete_form', $group_id, $response->data->name);
  }
  // The group does not exist. Return to the Groups management page.
  else {
    drupal_goto('springboard/advocacy/targets/custom-groups');
  }
  return $output;
}

function springboard_target_group_delete_form($form, &$form_state, $group_id, $group_name) {
  $form['group_id'] = array(
    '#type' => 'value',
    '#value' => $group_id,
  );
  $form['#submit'][] = 'springboard_target_group_delete_submit';

  $confirmation_question = t('Delete Group @name?', array('@name' => $group_name));
  $cancel_path = 'springboard/advocacy/targets/custom-groups';
  $description = t('Are you sure you want to delete this group? This cannot be undone.');
  $yes_button = t('Yes');
  $no_button = t('Cancel');
  $confirm_name = 'springboard_target_group_delete_submit';
  return confirm_form($form, $confirmation_question, $cancel_path, $description, $yes_button, $no_button, $confirm_name);
}

/**
 * Submit function for target deletion.
 *
 * @param $form
 * @param $form_state
 */
function springboard_target_group_delete_submit(&$form, &$form_state) {
  $form_state['redirect'] = 'springboard/advocacy/targets/custom-groups';

  $api_call = springboard_advocacy_api_call();
  $response = $api_call->invokeClientMethod('deleteTargetGroup', $form_state['values']['group_id']);

  if (isset($response->data)) {
    drupal_set_message('Group deleted.');
  }
}
