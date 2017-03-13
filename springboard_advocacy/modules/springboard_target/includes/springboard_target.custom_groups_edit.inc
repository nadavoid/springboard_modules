<?php
/**
 * @file
 * Functions for target updates.
 */

/**
 * Group edit form.
 *
 * Add edit form items to the generic form defined
 * in springboard_target.custom_groups.
 *
 * @param $form
 * @param $form_state
 * @param $group_id
 *
 * @return array
 *   The edit form.
 */
function springboard_target_edit_custom_groups_form($form, &$form_state, $group_id) {

  module_load_include('inc', 'springboard_target', 'includes/springboard_target.custom_groups');

  $form = springboard_target_group_form($group_id);

  if (!isset($form['not_found'])) {

    $form['header'] = array(
      '#prefix' => '<h2>',
      '#suffix' => '</h2>',
      '#markup' => check_plain($form['group_group_name']['#default_value']),
      '#weight' => '-100' ,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Update'),
      '#weight' => 10,
      '#attributes' => array('class' => 'group-add'),
    );
  }
  $form['#validate'][] = 'springboard_target_group_form_validate';

  return $form;

}

/**
 * Submit function for group edit form.
 *
 * @param $form
 * @param $form_state
 */
function springboard_target_edit_custom_groups_form_submit($form, &$form_state) {
  $id = $form_state['build_info']['args'][0];
  $input = $form_state['values'];
  $api_call = springboard_advocacy_api_call();

  // @TODO need a way to retrieve the target name via the ID
  //$group = $api_call->invokeClientMethod('updateTargetGroup', $input, $id);

  if (!empty($group->data)) {
    $message = implode(' ', (array) $group->data->status);
    drupal_set_message($message);
    $form_state['redirect'] = 'admin/springboard/advocacy/targets/custom-groups/';
  }
}
