<?php

/**
 * @file
 * Dashboard functions for Springboard advocacy.
 */

/**
 * Page callback for the advocacy dashboard.
 *
 * Embeds and themes various views landing pages and the target
 * creation/edit forms.
 *
 * @param string $path
 *   The drupal path.
 * @param string $arg
 *   Arg from url.
 *
 * @return mixed
 *   An array of page elements ready for theme.
 */
function springboard_advocacy_dashboard($path, $arg = NULL) {

  // If the API is not configured, intercept some displays and
  // show an error message.
  if ($path != 'actions' && $path != 'petitions/all' && $path != 'message-actions/all') {
    $unconfigured = springboard_advocacy_api_unconfigured('springboard');
    if (!empty($unconfigured)) {
      $items = implode(', ', $unconfigured);
      $message = t('Please <a href="!url">configure</a> the Springboard Advocacy API settings. There are empty values in :items. This feature will not function without required configuration.', array('!url' => url('admin/config/services/advocacy'), ':items' => $items));
      $sections = springboard_advocacy_dashboard_error('error', t('Configuration required'), $message);
      return theme('springboard_advocacy_dashboard', array('sections' => $sections));
    }
  }

  global $user;
  $sections  = array();
  // Prepare pages based on path.
  switch ($path) {

    // Custom Target edit form page.
    case 'add-custom-targets':
    case 'edit-custom-targets':
    case 'add-custom-groups':
    case 'edit-custom-groups':
      $sections = springboard_advocacy_custom_targets_admin_form($path, $arg);
      break;

    // Target Center landing page view.
    case 'target-center':
      // Get the views.
      $targets_view = springboard_advocacy_prepare_dashboard_view('sba_custom_targets', 'block_1');
      $groups_view = springboard_advocacy_prepare_dashboard_view('sba_custom_targets', 'block_2'); // @TODO - create this view display and update this id if neccessary.
      // The view is not found or the display is not found.
      if (!$targets_view) {
        $message = t('Target view or view display "block_1" not found.');
        $sections = springboard_advocacy_dashboard_error('sba_targets', t('Target Center'), $message);
        break;
      }

      if (!$groups_view) {
        $message = t('Groups view or view display "block_2" not found.');
        $sections = springboard_advocacy_dashboard_error('sba_targets', t('Target Center'), $message);
        break;
      }

      $sections = array(
        'sba_targets' => array(
          'header' => t('Custom Targets'),
          'content' => $targets_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
          'manage_link' => array('title' => t('View All Custom Targets'), 'href' => 'admin/springboard/advocacy/targets/custom-targets'),
        ),
        'sba_groups' => array(
          'header' => t('Custom Groups'),
          'content' => $groups_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
          'manage_link' => array('title' => t('View all Target Groups'), 'href' => 'admin/springboard/advocacy/targets/custom-groups'),
        ),
      );

      if (module_exists('springboard_target') && springboard_target_target_access('create targets')) {
        $sections['sba_targets']['create_link'] = array('title' => t('Create a new target'), 'href' => 'admin/springboard/advocacy/targets/custom-targets/add');
      }
      if (module_exists('springboard_target') && springboard_target_target_access('create groups')) {
        $sections['sba_groups']['create_link'] = array('title' => t('Create a new group'), 'href' => 'admin/springboard/advocacy/targets/custom-groups/add');
      }
      break;

    // Custom Targets listing page view.
    case 'custom-targets':
      // Get the view.
      $targets_view = springboard_advocacy_prepare_dashboard_view('sba_custom_targets', 'block_1');
      // The view is not found or the display is not found.
      if (!$targets_view) {
        $message = t('Target view or view display "block_1" not found.');
        $sections = springboard_advocacy_dashboard_error('sba_targets', t('Custom Targets'), $message);
        break;
      }

      $sections = array(
        'sba_targets' => array(
          'header' => t('Custom Targets'),
          'content' => $targets_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
        ),
      );
      if (module_exists('springboard_target') && springboard_target_target_access('create targets')) {
        $sections['sba_targets']['create_link'] = array('title' => t('Create a new target'), 'href' => 'admin/springboard/advocacy/targets/custom-targets/add');
      };
      break;

    // Custom Groups listing page view.
    case 'custom-groups':
      // Get the view.
      $groups_view = springboard_advocacy_prepare_dashboard_view('sba_custom_targets', 'block_2'); // @TODO - create this view display and update this id if neccessary.
      // The view is not found or the display is not found.
      if (!$groups_view) {
        $message = t('Groups view or view display "block_2" not found.');
        $sections = springboard_advocacy_dashboard_error('sba_targets', t('Custom Groups'), $message);
        break;
      }

      $sections = array(
        'sba_groups' => array(
          'header' => t('Custom Groups'),
          'content' => $groups_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
        ),
      );
      if (module_exists('springboard_target') && springboard_target_target_access('create groups')) {
        $sections['sba_groups']['create_link'] = array('title' => t('Create a new group'), 'href' => 'admin/springboard/advocacy/targets/custom-groups/add');
      };
      break;

    // Actions landing page view. Includes Actions and Petitions.
    // Since sections here are optional, don't set an error message
    // if one is missing.
    case 'actions':

      if (user_access('create sba_message_action content') || user_access('edit any sba_message_action content')) {
        // Actions.
        $actions_view = springboard_advocacy_prepare_dashboard_view('sba_actions', 'block_3', array('sba_message_action'));
        if ($actions_view && module_exists('sba_message_action')) {
          $sections = array(
            'sba_actions' => array(
              'header' => t('Message Actions'),
              'content' => $actions_view->preview(),
              'viewmore_link' => array(
                'title' => t('View All Message Actions'),
                'href' => 'admin/springboard/advocacy/message-actions/all'
              ),
              'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
            ),
          );
          if (user_access('create sba_message_action content')) {
            $sections['sba_actions']['create_link'] = array(
              'title' => t('Create a new message action'),
              'href' => 'node/add/sba-message-action',
            );
          }
          if (module_exists('springboard_target') && (springboard_target_user_can_access_targets() || springboard_target_user_can_access_groups())) {
            $sections['sba_actions']['manage_link'] = array(
              'title' => t('Manage Custom Targets'),
              'href' => 'admin/springboard/advocacy/targets',
            );
          }
        }
      }

      if (user_access('create sba_social_action content') || user_access('edit any sba_social_action content')) {

        // Social Actions.
        $social_actions_view = springboard_advocacy_prepare_dashboard_view('sba_actions', 'block_3', array('sba_social_action'));
        if ($social_actions_view && module_exists('sba_social_action')) {
          $sections['sba_social_actions'] = array(
            'header' => t('Social Actions'),
            'content' => $social_actions_view->preview(),
            'viewmore_link' => array(
              'title' => t('View All Social Actions'),
              'href' => 'admin/springboard/advocacy/social-actions/all',
            ),
            'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
          );
          if (user_access('create sba_social_action content')) {
            $sections['sba_social_actions']['create_link'] = array(
              'title' => t('Create a new social action'),
              'href' => 'node/add/sba-social-action',
            );
          }
          if (module_exists('springboard_target') && (springboard_target_user_can_access_targets() || springboard_target_user_can_access_groups())) {
            $sections['sba_social_actions']['manage_link'] = array(
              'title' => t('Manage Custom Targets'),
              'href' => 'admin/springboard/advocacy/targets',
            );
          }
        }
      }

      if (user_access('create springboard_petition content') || user_access('edit any springboard_petition content')) {
        $petitions_view = springboard_advocacy_prepare_dashboard_view('sba_actions', 'block_1', array('springboard_petition'));
        if ($petitions_view && module_exists('springboard_petition')) {
          $sections['sba_petitions'] = array(
            'header' => t('Petitions'),
            'content' => $petitions_view->preview(),
            'viewmore_link' => array(
              'title' => t('View All Petitions'),
              'href' => 'admin/springboard/advocacy/petitions/all',
            ),
            'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
          );
          if (user_access('create springboard_petition content')) {
            $sections['sba_petitions']['create_link'] = array(
              'title' => t('Create a new petition'),
              'href' => 'node/add/springboard-petition',
            );
          }
        }
      }
      break;

    // All message actions view page.
    case 'message-actions/all':
      $actions_all_view = springboard_advocacy_prepare_dashboard_view('sba_actions', 'block_4', array('sba_message_action'));
      if (!$actions_all_view) {
        $sections = springboard_advocacy_dashboard_error('sba_actions', t('All Message Actions'), t('Message Actions view or view display "block_2" not found.'));
        break;
      }

      $sections = array(
        'sba_actions' => array(
          'header' => t('All Message Actions'),
          'content' => $actions_all_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
        ),
      );

      if (user_access('create sba_message_action content')) {
        $sections['sba_actions']['create_link'] = array('title' => t('Create a new message action'), 'href' => 'node/add/sba-message-action');
      }
      if (springboard_target_user_can_access_targets() || springboard_target_user_can_access_groups()) {
        $sections['sba_actions']['manage_link'] = array('title' => t('Manage Custom Targets'), 'href' => 'admin/springboard/advocacy/targets');
      }
      break;

    case 'social-actions/all':
      $social_actions_all_view = springboard_advocacy_prepare_dashboard_view('sba_actions', 'block_4', array('sba_social_action'));
      if (!$social_actions_all_view) {
        $sections = springboard_advocacy_dashboard_error('sba_actions', t('All Message Actions'), t('Message Actions view or view display "block_2" not found.'));
        break;
      }

      $sections['sba_social_actions'] = array(
          'header' => t('All Social Actions'),
          'content' => $social_actions_all_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
      );

      if (user_access('create sba_message_action content')) {
        $sections['sba_social_actions']['create_link'] = array('title' => t('Create a new social action'), 'href' => 'node/add/sba-social-action');
      }
      if (module_exists('springboard_target') && (springboard_target_user_can_access_targets() || springboard_target_user_can_access_groups())) {
        $sections['sba_social_actions']['manage_link'] = array('title' => t('Manage Custom Targets'), 'href' => 'admin/springboard/advocacy/targets');
      }

      break;
    // All Petitions view page.
    case 'petitions/all':
      $petitions_all_view = springboard_advocacy_prepare_dashboard_view('sba_actions', 'block_2', array('springboard_petition'));
      if ($petitions_all_view) {
        $sections = array(
          'sba_actions' => array(
            'header' => t('All Petitions'),
            'content' => $petitions_all_view->preview(),
            'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
          ),
        );
        if (user_access('create springboard_petition content')) {
          $sections['sba_actions']['create_link'] = array('title' => t('Create a new petition'), 'href' => 'node/add/springboard-petition');
        }
      }
      else {
        $sections = springboard_advocacy_dashboard_error('sba_actions', t('All Petitions'), t('Petitions view or view display "block_2" not found.'));
      }
      break;

    // node/%/messages message landing page.
    case ($path == arg(1)):

      $node = node_load($path);

      // Is this a message type?
      $message_types = variable_get('sba_message_node_types', array());
      if (!isset($message_types[$node->type])) {
        $sections = springboard_advocacy_dashboard_error('sba_messages_node', t('Messages'), t('There was a problem locating the message type for this node type.'));
        break;
      }

      // Get the view.
      if ($node->type == 'sba_message_action') {
        $messages_view = springboard_advocacy_prepare_dashboard_view('sba_messages_node', 'block_1', array($path));
      }
      else {
        $messages_view = springboard_advocacy_prepare_dashboard_view('sba_messages_node', 'social_messages_tab', array($path));
      }

      // Couldn't load the correct view?
      if (!$messages_view) {
        $error_message = t('Message view or view display "block_1" not found');
        $sections = springboard_advocacy_dashboard_error('sba_messages_node', t('Messages'), $error_message);
        break;
      }

      // Get the messages for this node and count them.
      if (isset($node->message_ids)) {
        $messages = entity_load('sba_message', $node->message_ids);
        // If there's only one message, and it's user-editable
        // prevent display of create message link.
        if (count($messages) == 1) {

          // Hide the "show row weights" and weight column
          // if there's only one message.
          drupal_add_js('jQuery(document).ready(function () {if (jQuery(".views-row-first.views-row-last").length == 1) {jQuery(".tabledrag-toggle-weight-wrapper").hide();jQuery.cookie("Drupal.tableDrag.showWeight", 0, {path: Drupal.settings.basePath,expires: 365});}})', array('type' => 'inline'));

          $message = array_pop($messages);
          if ($node->type == 'sba_message_action') {
            $subject_editable = field_get_items('sba_message', $message, 'field_sba_subject_editable');
            $user_editable = field_get_items('sba_message', $message, 'field_sba_user_editable');
            $multi_flow = field_get_items('node', $node, 'field_sba_action_flow');
            if ($multi_flow[0]['value'] != 'multi' && ($subject_editable[0]['value'] != 'Not editable' || !empty($user_editable))) {
              $no_create = TRUE;
              drupal_set_message(t("This single-step action's current message is user-editable, please disable user editing or use the multi-step action flow to create additional messages."), 'error', FALSE);
            }
          }
        }
      }


      // Set the content.
      $sections = array(
        'sba_messages_node' => array(
          'header' => t('Messages'),
          'content' => $messages_view->preview(),
          'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
        ),
      );

      // Display/hide create link.
      if ((user_access('edit own sba_message_action content') && $user->uid == $node->uid) || user_access('edit any sba_message_action content')) {
        $sections['sba_messages_node']['create_link'] = array('title' => t('Create a new message'), 'href' => 'node/' . arg(1) . '/messages/add/' . $message_types[$node->type]);
      }

      if (!empty($no_create)) {
        unset($sections['sba_messages_node']['create_link']);
      }
      break;

    default:
      $sections = array();
  }

  return theme('springboard_advocacy_dashboard', array('sections' => $sections));
}

/**
 * Advocacy dashboard Custom Targets & Groups form page helper.
 *
 * @param string $path
 *   The drupal path.
 * @param string $arg
 *   The url arg.
 *
 * @return array
 *   An array to be themed.
 */
function springboard_advocacy_custom_targets_admin_form($path, $arg = NULL) {

  if (!module_exists("springboard_target")) {
    $sections = springboard_advocacy_dashboard_error('sba_target', t('Custom Targets'), t('Springboard Target Module is not enabled.'));
    return $sections;
  }

  drupal_add_css(drupal_get_path('module', 'springboard_target') . '/css/sba_target.css');

  switch($path) {
    case 'edit-custom-targets':
      module_load_include('inc', 'springboard_target', 'includes/springboard_target.custom_targets_edit');
      $form = drupal_get_form('springboard_target_edit_custom_targets_form', $arg);
      $header = t('Edit Target');
      $manage_link = array('title' => t('Return to targets list'), 'href' => 'admin/springboard/advocacy/targets/custom-targets');
      break;

    case 'edit-custom-groups':
      module_load_include('inc', 'springboard_target', 'includes/springboard_target.custom_groups_edit');
      $form = drupal_get_form('springboard_target_edit_custom_groups_form', $arg); //@TODO define this form
      $header = t('Edit Group');
      $manage_link = array('title' => t('Return to groups list'), 'href' => 'admin/springboard/advocacy/targets/custom-groups');
      break;

    case 'add-custom-groups':
      module_load_include('inc', 'springboard_target', 'includes/springboard_target.custom_groups_create');
      $form = drupal_get_form('springboard_target_create_custom_groups_form'); //@TODO define this form
      $header = t('Create Group');
      $manage_link = array('title' => t('Return to groups list'), 'href' => 'admin/springboard/advocacy/targets/custom-groups');
      break;

    default:
      module_load_include('inc', 'springboard_target', 'includes/springboard_target.custom_targets_create');
      $form = drupal_get_form('springboard_target_create_custom_targets_form');
      $header = t('Create Target');
      $manage_link = array('title' => t('Return to targets list'), 'href' => 'admin/springboard/advocacy/targets/custom-targets');
      break;
  }

  $sections = array(
    'target_form' => array(
      'header' => $header,
      'content' => drupal_render($form),
      'manage_link' => $manage_link,
      'css' => drupal_get_path('module', 'springboard_advocacy') . '/css/sba.css',
    ),
  );
  if (springboard_target_target_access('create targets') && isset($arg)) {
    if($path == 'edit-custom-targets') {
      $sections['target_form']['create_link'] = array(
        'title' => t('Create a new target'),
        'href' => 'admin/springboard/advocacy/targets/custom-targets/add'
      );
    }
    elseif ($path == 'edit-custom-groups') {
      $sections['target_form']['create_link'] = array(
        'title' => t('Create a new group'),
        'href' => 'admin/springboard/advocacy/targets/custom-groups/add'
      );
    }
  };

  return $sections;
}

/**
 * Advocacy dashboard views constructor.
 *
 * Will return false if view or view display is missing.
 *
 * @param string $name
 *   The view name.
 * @param string $display
 *   The view display.
 * @param array $args
 *   The view args.
 *
 * @return bool|view
 *   FALSE, or a view object.
 */
function springboard_advocacy_prepare_dashboard_view($name, $display, $args = array()) {
  $view = views_get_view($name);

  if (empty($view)) {
    return FALSE;
  }

  if (isset($view->display[$display])) {
    $view->set_display($display);
  }
  else {
    return FALSE;
  }

  if (!empty($args)) {
    $view->set_arguments($args);
  }

  $view->pre_execute();

  $view->execute();

  if ($view->access($display)) {
    return $view;
  }
  else {
    return FALSE;
  }
}

/**
 * Format a dashboard error message.
 *
 * @param string $section_name
 *   The page section name.
 * @param string $title
 *   The section title.
 * @param string $message
 *   The error message.
 *
 * @return array
 *   An array ready for rendering.
 */
function springboard_advocacy_dashboard_error($section_name, $title, $message) {
  $sections = array(
    '{$section_name}' => array(
      'header' => $title,
      'content' => '<div>' . $message . '</div>',
    ),
  );
  return $sections;
}
