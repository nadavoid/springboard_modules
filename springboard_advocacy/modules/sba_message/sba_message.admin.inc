<?php

/**
 * @file
 * SBA Message editing UI.
 */

/**
 * UI controller.
 */
class SbaMessageUIController extends EntityDefaultUIController {
  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {

    $items = array();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%' . $this->entityType;
    // @todo the menu type and context definitions here are definitely
    // not "correct" but they do allow the tabs to remain consistent
    // throughout editing workflow.
    foreach (sba_message_get_types() as $type) {
      //node/*/messages/add/*
      $items[$this->path . '/add/' . $type->type] = array(
        'title' => 'Add ' . $type->label,
        'page callback' => 'drupal_get_form',
        'page arguments' => array('sba_message_edit_form', $type->type),
        'access callback' => 'sba_message_access',
        'access arguments' => array('create', 'create ' . $type->type),
        'file' => 'sba_message.admin.inc',
        'file path' => drupal_get_path('module', $this->entityInfo['module']),
        'type' => MENU_DEFAULT_LOCAL_TASK,
        'context' => MENU_CONTEXT_INLINE,
      );
    }

    // Loading and editing sba_message entities.
    // node/*/messages/sba_message/*/edit
    $items[$this->path . '/sba_message/' . $wildcard] = array(
      'page callback' => 'drupal_get_form',
      'page arguments' => array('sba_message_edit_form', $id_count + 1),
      'access callback' => 'sba_message_access',
      'access arguments' => array('edit', $id_count + 2),
      'weight' => 0,
      'file' => 'sba_message.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
      'type' => MENU_LOCAL_TASK,
    );
    $items[$this->path . '/sba_message/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
    );
    $items[$this->path . '/sba_message/' . $wildcard . '/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'sba_message_delete_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'sba_message_access',
      'access arguments' => array('delete', $id_count + 1),
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_INLINE,
      'weight' => 10,
      'file' => 'sba_message.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module']),
    );
    return $items;
  }
}

/**
 * Form callback wrapper: delete a sba_message.
 *
 * @param $sba_message
 *   The sba_message object being edited by this form.
 *
 * @return mixed
 *   A rendered delete from.
 *
 * @see sba_message_edit_form()
 */
function sba_message_delete_form_wrapper($sba_message) {
  // Add the breadcrumb for the form's location.
  return drupal_get_form('sba_message_delete_form', $sba_message);
}

/**
 * Message create/edit form.
 *
 * This is where all the targeting elements are added.
 *
 * @param $form
 *   The form array.
 * @param $form_state
 *   The form meta-data and values.
 * @param $sba_message
 *   The message node being edited.
 * @param $node
 *   The parent sba_message_action node.
 *
 * @return mixed
 */
function sba_message_edit_form($form, &$form_state, $sba_message) {

  if (is_string($sba_message)) {
    $sba_message = sba_message_create(array('type' => $sba_message));
  }
  $node = node_load(arg(1));
  $unconfigured = springboard_advocacy_api_unconfigured('springboard');
  if (!$unconfigured) {
    $apiCall = springboard_advocacy_api_call();
    $apiCall->invokeClientMethod('getMaintenanceMode');
  }
  elseif(user_access('administer springboard advocacy')) {
    $items = implode(', ', $unconfigured);
    $message = t('Please <a href="!url">configure</a> the Springboard Advocacy API settings. There are empty values in :items. This feature will not function without required configuration.', array('!url' => url('admin/config/services/advocacy'), ':items' => $items));
    drupal_set_message($message, 'error');
  }

  if(!empty($node)) {
    $form_state['storage']['node'] = $node;
  }
  else {
    drupal_set_message(t('Node object could not be loaded.'), 'error');
  }

  $form = array(
    '#prefix' => '<div id="springboard-advocacy-message-form-container">',
    '#suffix' => '</div>',
  );

  // Add the field related form elements.
  $form_state['sba_message'] = $sba_message;
  field_attach_form('sba_message', $sba_message, $form, $form_state);
  $form['parent_node']['nid'] = array(
    '#type' => 'hidden',
    '#value' => $form_state['storage']['node']->nid,
  );
  $form['advocacy_id'] = array(
    '#type' => 'hidden',
    '#value' => $form_state['storage']['node']->advocacy_id,
  );
  // Add the default field elements.
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Internal Title'),
    '#default_value' => isset($sba_message->name) ? $sba_message->name : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -5,
  );
  
  $form['data']['#tree'] = TRUE;
  // Get/Set the message UUID.
  $form['data']['message_id'] = array(
    '#type' => 'hidden',
    '#value' => isset($sba_message->data['message_id']) ? $sba_message->data['message_id'] : str_replace(' ', '-', uniqid(substr(variable_get('site_name', ''), 0, 10) . '-' . substr($node->title, 0, 10) . '-')),
  );

  $form['data']['group_id'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($sba_message->data['group_id']) ? $sba_message->data['group_id'] : '',
  );

  $form['data']['precedence'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($node->precedence) ? $node->precedence : 0,
  );

  if (empty($sba_message->is_new)) {
    $form['data']['weight'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($sba_message->data['weight']) ? $sba_message->data['weight'] : NULL,
    );
  }

  if (isset($sba_message->data['group_id']) && $sba_message->data['group_id'] == 'no_response') {
    drupal_set_message('A remote group ID could not be found.');
    watchdog('advocacy', 'A remote group ID could not be found. Local ID: @message', array('@message' => $sba_message->data['group_id']), WATCHDOG_ERROR);
  }

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 400,
  );

  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['message_error'] = array(
    '#markup' => '',
    '#prefix' => '<div id = "advo-error-wrapper">',
    '#suffix' => '</div>',
    '#weight' => 10,
  );

  // The introduction of the multiple twitter message ajax form
  // made this necessary. It is triggered by javascript
  // click on $form['actions']['submit']
  $form['submit_hidden'] = array(
    '#type' => 'submit',
    '#value' => t('Save message'),
    '#submit' => $submit + array('sba_message_edit_form_submit'),
    '#attributes' => array('style' => 'display:none'),
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save message'),
    '#submit' => $submit + array('sba_message_edit_form_submit'),
  );

  if(module_exists('sba_target_search')) {
    // Add the target search and elements for saving targets.
    $form = sba_target_search_add_target_search_form($form, $form_state, $sba_message);
    // If the message current has no recipients, give the option to save the
    // recipients as a re-usable custom group.
    $recipients = isset($sba_message->data['recipients']) ? json_decode($sba_message->data['recipients'], TRUE) : '';
    $form['save_target_group_toggle_val'] = array(
      '#type' => 'hidden',
      '#value' => '',
      '#attributes' => array('id' => 'edit-save-target-group-save-target-group-toggle_val'),
    );
    $form['save_target_group_name_val'] = array(
      '#type' => 'hidden',
      '#value' => '',
      '#attributes' => array('id' => 'edit-save-target-group-save-target-group-name_val'),

    );
    if ($sba_message->type != 'sba_social_action_twitter_message') {
      $form['save_target_group'] = array(
        '#tree' => TRUE,
        '#type' => 'container',
        '#title' => 'save target group',
        '#weight' => 99,
      );
      $form['save_target_group']['save_target_group_toggle'] = array(
        '#type' => 'checkbox',
        '#title' => t('Save these targets as a custom group.'),
        '#weight' => 0,
        '#value' => !empty($form_state['input']['save_target_group_toggle_val']) ? 1 : 0,
      );
      // If recipients are already set, hide the option to save as target group.
      if (!empty($recipients) && empty($form_state['input']['save_target_group_toggle_val'])) {
        $form['save_target_group']['#attributes']['class'] = array('hidden');
      }
      $form['save_target_group']['save_target_group_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Group name'),
        '#weight' => 1,
        '#states' => array(
          'invisible' => array(
            ':input[name="save_target_group[save_target_group_toggle]"]' => array('checked' => FALSE),
          ),
          'required' => array(
            ':input[name="save_target_group[save_target_group_toggle]"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['#validate'][] = 'sba_message_save_target_group_validate';
    }
  }


  $form['#attached']['js'][] = drupal_get_path('module', 'sba_message') . '/js/sba_message.js';
  $form['#attached']['css'][] = drupal_get_path('module', 'sba_message') . '/css/sba_message.css';
  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'sba_message_edit_form_validate';
  return $form;
}

/**
 * Custom validation callback to save recipients as a custom group.
 */

function sba_message_save_target_group_validate(&$form, &$form_state) {
  $values = $form_state['input'];
  if($values['save_target_group_toggle_val'] == 1) {
    if($values['save_target_group_name_val'] == '') {
      form_set_error('save_target_group_name', t('Please provide a name for your custom group.'));
    }
    else if(!isset($values['data']['recipients']) || $values['data']['recipients'] == '') {
      form_set_error('save_target_group', t('Please add targets to save as a custom group.'));
    }
    else {
      // Build a group object for saving
      $group = array(
        'recipients' => $values['data']['recipients'],
        'name' => $values['save_target_group_name_val'],
        'is_template' => 1,
      );
      module_load_include('inc', 'springboard_target', 'includes/springboard_target.api_calls');
      $group_id = springboard_target_api_save_target_group($group, true);
      if ($group_id) {
        // Replace the recipients list with the new group.
        $recipients[] = array(
          'id' => $group_id,
          'type' => 'group',
          'name' => $values['save_target_group_name_val'],
          );
        $form_state['values']['data']['recipients'] = json_encode($recipients);
      }
      else {
        form_set_error('save_target_group', t('There was a problem saving the Group to the Advocacy server. Try again or uncheck "Save these targets as custom group"'));
      }
    }
  }
}

/**
 * Form API validate callback for the sba_message form.
 *
 * @param $form
 *   The form array.
 * @param $form_state
 *    The form meta-data and values.
 */
function sba_message_edit_form_validate(&$form, &$form_state) {
  $sba_message = $form_state['sba_message'];
  // Notify field widgets to validate their data.
  field_attach_form_validate('sba_message', $sba_message, $form, $form_state);
}


/**
 * Form API submit callback for the sba_message form.
 *
 * @param $form
 *   The form array.
 * @param $form_state
 *    The form meta-data and values.
 */
function sba_message_edit_form_submit(&$form, &$form_state) {
  $sba_message = entity_ui_controller('sba_message')->entityFormSubmitBuildEntity($form, $form_state);
  // Add in created and changed times.
  $is_new = FALSE;
  if ($sba_message->is_new = isset($sba_message->is_new) ? $sba_message->is_new : 0) {
    $sba_message->created = time();
    $is_new = TRUE;
  }

  $sba_message->changed = time();

  $status = $sba_message->save();
  if ($status) {
    drupal_set_message(t('Message saved.'));
  }

  $wrapper = entity_metadata_wrapper('sba_message', $sba_message);
  $nid = $wrapper->field_sba_action_id->value()->nid;
  // Set a weight and an entry in the draggableviews table
  // if there is no weight.
  if (!isset($sba_message->data['weight']) && isset($sba_message->sba_message_id)) {
    $arg = json_encode(array($nid));
    // Create new order record.
    $weight = db_query("SELECT MAX(weight) FROM {draggableviews_structure} WHERE view_name = :name and args = :arg",
      array(':name' => 'sba_messages_node', ':arg' => $arg))->fetchField();

    $sba_message->data['weight']  = $weight + 1;
    $sba_message->save();

    sba_message_draggable_save($arg, $sba_message->sba_message_id, $sba_message->data['weight']);
  }

  $values = $form_state['values'];
  if(isset($values['data']['recipients'])) {
    // Build a group object for saving
    $group = array(
      'group_id' => $sba_message->data['group_id'],
      'recipients' => $values['data']['recipients'],
      'name' => $values['data']['message_id'],
      'is_template' => 0,
      'message' => array(
        'group_id' => $sba_message->data['group_id'],
        'message_id' => $values['data']['message_id'],
        'form_id' => $values['advocacy_id'],
      ),
    );
    module_load_include('inc', 'springboard_target', 'includes/springboard_target.api_calls');
    $group_id = springboard_target_api_save_target_group($group, $is_new);
    $sba_message->data['recipients'] = html_entity_decode($sba_message->data['recipients']);
    if ($group_id) {
      $sba_message->data['group_id'] = $group_id;
      $sba_message->changed = time();
      $sba_message->save();
    }
    else {
      $sba_message->data['group_id'] = 'no_response';
      drupal_set_message('There was a problem saving the message targets to the advocacy server.', 'warning');
      $sba_message->changed = time();
      $sba_message->save();
    }
  }

  $form_state['redirect'] = 'node/' . $nid . '/messages';
}


/**
 * Form API submit callback for the delete button.
 *
 * @param $form
 *   The form array.
 * @param $form_state
 *    The form meta-data and values.
 */
function sba_message_form_submit_delete(&$form, &$form_state) {
  $nid =  $form_state['values']['nid'];
  $form_state['redirect'] = 'node/' . $nid . '/messages/sba_message/' . $form_state['sba_message']->sba_message_id . '/delete';
}


/**
 * Form callback: confirmation form for deleting a sba_message.
 *
 * @see confirm_form()
 *
 * @param $form
 *   The form array.
 * @param $form_state
 *    The form meta-data and values.
 * @param $sba_message  The sba_message to delete
 *
 * @return mixed
 */
function sba_message_delete_form($form, &$form_state, $sba_message) {
  $form_state['sba_message'] = $sba_message;
  $wrapper = entity_metadata_wrapper('sba_message', $sba_message);
  $nid = $wrapper->field_sba_action_id->value()->nid;

  $form['#submit'][] = 'sba_message_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete sba_message %name?', array('%name' => $sba_message->name)),
    'node/' . $nid . '/messages/sba_message',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );
  
  return $form;
}

/**
 * Submit callback for sba_message_delete_form
 * @param $form
 *   The form array.
 * @param $form_state
 *    The form meta-data and values.
 */
function sba_message_delete_form_submit($form, &$form_state) {
  // Delete the message.
  $sba_message = $form_state['sba_message'];
  $wrapper = entity_metadata_wrapper('sba_message', $sba_message);
  $nid = $wrapper->field_sba_action_id->value()->nid;
  sba_message_delete($sba_message);
  drupal_set_message(t('%name has been deleted.', array('%name' => $sba_message->name)));
  watchdog('sba_message', 'Deleted sba_message: %name.', array('%name' => $sba_message->name));
  form_load_include($form_state, 'inc', 'sba_message', 'sba_message.api_calls');

  if(isset($sba_message->data['group_id']) && $sba_message->data['group_id'] != 'no_response') {
    module_load_include('inc', 'springboard_target', 'includes/springboard_target.api_calls');
    springboard_target_api_delete_target_group($sba_message);
  }

  $form_state['redirect'] = 'node/' . $nid . '/messages';
}
