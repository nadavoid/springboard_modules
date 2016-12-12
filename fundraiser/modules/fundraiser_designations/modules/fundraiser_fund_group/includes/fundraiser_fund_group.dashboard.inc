<?php

/**
 * @file
 * Dashboard functions for fund groups.
 */

/**
 * Page callback for the fund groups dashboard.
 *
 * Embeds and themes various views landing pages and the target
 * creation/edit forms.
 *
 * @param object $node
 *   The form node.
 * @param string $arg
 *   Arg from url.
 *
 * @return mixed
 *   An array of page elements ready for theme.
 */
function fundraiser_fund_group_dashboard($node, $arg = NULL) {

  global $user;

  // Is this a designation type?
  $designation_types = fundraiser_designations_get_designation_types();
  if (!isset($designation_types[$node->type])) {
    $sections = fundraiser_fund_group_dashboard_error('fundraiser_fund_group_node', t('Messages'), t('There was a problem locating the message type for this node type.'));
    return theme('fundraiser_fund_group_dashboard', array('sections' => $sections));
  }

  // Get the view.
  $groups_view = fundraiser_fund_group_prepare_dashboard_view('fundraiser_fund_group', 'block_1', array($node->nid));

  // Get the messages for this node and count them.
  if (isset($node->group_ids)) {
    $groups = entity_load('fundraiser_fund_group', $node->group_ids);
    // If there's only one group, and it's user-editable
    // prevent display of create group link.
    if (count($groups) == 1) {
      // Hide the "show row weights" and weight column
      // if there's only one group.
      drupal_add_js('jQuery(document).ready(function () {if (jQuery(".views-row-first.views-row-last").length == 1) {jQuery(".tabledrag-toggle-weight-wrapper").hide();jQuery.cookie("Drupal.tableDrag.showWeight", 0, {path: Drupal.settings.basePath,expires: 365});}})', array('type' => 'inline'));
    }
  }

  // Set the content.
  $sections = array(
    'fundraiser_fund_group_node' => array(
      'header' => t('Designation Groups'),
      'content' => $groups_view->preview(),
      'css' => drupal_get_path('module', 'fundraiser_fund_groups') . '/css/fund-groups.css',
    ),
  );

  // Display/hide create link.
  if ((user_access('edit own fundraiser_fund_group_action content') && $user->uid == $node->uid) || user_access('edit any fundraiser_fund_group_action content')) {
    $sections['fundraiser_fund_group_node']['create_link'] = array('title' => t('Create a new group'), 'href' => 'node/' . arg(1) . '/designations/add/fundraiser_fund_group_default');
  }

  if (!empty($no_create)) {
    unset($sections['fundraiser_fund_group_node']['create_link']);
  }

  return theme('fundraiser_fund_group_dashboard', array('sections' => $sections));
}


/**
 * FUnd groups dashboard views constructor.
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
function fundraiser_fund_group_prepare_dashboard_view($name, $display, $args = array()) {
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
 * Format a dashboard error group.
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
function fundraiser_fund_group_dashboard_error($section_name, $title, $message) {
  $sections = array(
    '{$section_name}' => array(
      'header' => $title,
      'content' => '<div>' . $message . '</div>',
    ),
  );
  return $sections;
}
