<?php
/**
 * @file
 * Page wrapper functions for springboard groups.
 */

/**
 * Implements hook_page_wrappers_node_settings_form_wrappers_alter().
 *
 * Alter the wrappers options if a node is grouped.
 */
function springboard_group_page_wrappers_node_settings_form_wrappers_alter(&$wrappers, $form, $form_state) {
  $wrappers = springboard_group_page_wrappers_filter($wrappers, $form, $form_state);
}

/**
 * Alter a wrapped node's edit form.
 *
 * A. Add a vaildate function to verify group selection on submission.
 * B. Rebuild the form if an ajax callback has fired and the group has changed.
 * C. When editing a page wrapper itself, validate that switching groups won't
 * cause a conflict.
 */
function springboard_group_page_wrappers_node_form__alter(&$form, &$form_state, $callback) {
  // We're editing a *wrapped* node and need to validate the wrapper's groups.
  if ($form['#node']->type != 'page_wrapper') {
    $form['#validate'][] = 'springboard_group_page_wrappers_validate_submission';
  }
  // Ajax reload has occurred.
  if (!empty($form_state['values']['page_wrappers']) && $form['#node']->type != 'page_wrapper') {
    if ($callback != 'springboard_group_og_group_field_ajax_callback' &&  $callback != '_page_wrappers_add_js') {
      return;
    }
    // Get the updated wrappers list.
    $wrappers = _page_wrappers_available_wrappers();
    $wrappers_filtered = springboard_group_page_wrappers_filter($wrappers, $form, $form_state);
    // Rebuild the form.
    page_wrappers_build_node_form($form, $form_state, $wrappers_filtered);
    // Show a message about outdated wrappers if necessary.
    springboard_group_page_wrappers_validate_group_switch($form, $form_state);
  }
  // We on a page *wrapper* and need to validate nodes that are
  // already wrapped.
  elseif (!empty($form_state['values']) && $form['#node']->type == 'page_wrapper') {
    springboard_group_wrappers_common_validation_helper($form, $form_state, 'node_page_wrappers');
  }
}

/**
 * Validate page wrappers used on a node.
 *
 * Check if a previously configured wrapper is compatible with the new group
 * when switching groups on a wrapped node. Show a message if it isn't.
 */
function springboard_group_page_wrappers_validate_group_switch(&$form, &$form_state) {
  $x = 0;
  $lost_wrappers = [];
  foreach ($form_state['values']['page_wrappers'] as $key => $wrapper) {
    $current_wrapper = !empty($form_state['values']['page_wrappers'][$key]['wrapper_nid']) ? $form_state['values']['page_wrappers'][$key]['wrapper_nid'] : NULL;
    $default = !empty($form['#node']->page_wrappers[$x]['wrapper_nid']) ? $form['#node']->page_wrappers[$x]['wrapper_nid'] : NULL;
    $options = $form['springboard_display']['page_wrappers_settings']['page_wrappers'][$key]['wrapper_nid']['#options'];
    if (!in_array($current_wrapper, array_keys($options))) {
      $lost_wrappers[] = $current_wrapper;
    }
    elseif (in_array($default, array_keys($options))) {
      $form_state['input']['page_wrappers'][$key]['wrapper_nid'] = $default;
    }
    $x++;
  }
  if (!empty($lost_wrappers)) {
    $num = count($lost_wrappers);
    $wrp_fmt = format_plural(count($lost_wrappers), 'wrapper', 'wrappers');
    $list = [];
    foreach ($lost_wrappers as $wrapper) {
      $wrapper_node = node_load($wrapper);
      if (!empty($wrapper_node->title)) {
        $list[] = l(check_plain($wrapper_node->title), 'node/' . $wrapper_node->nid, array('attributes' => array('target' => '_blank')));
      }
    }
    $wrapper_message = theme('item_list', array('items' => $list));
    $form_state['group_ajax_messages']['page_wrappers'] = t('@num page @wrp_fmt selection will be removed: !wrapper_message', array(
      '@num' => $num,
      '@wrp_fmt' => $wrp_fmt,
      '!wrapper_message' => $wrapper_message,
    ));
  }
}

/**
 * Validate page wrappers on non-ajax form submission.
 *
 * If an invaild pagewrapper has been selected, somehow, show and error and
 * rebuild the page wrapper selection form.
 */
function springboard_group_page_wrappers_validate_submission(&$form, &$form_state) {
  if (!empty($form_state['triggering_element']['#ajax']['callback'])) {
    return;
  }

  $gids = springboard_group_get_gids_from_element($form, $form_state);
  $allowed_wrappers = springboard_group_page_wrappers_list_wrappers_by_group($gids);
  $error = FALSE;
  foreach ($form_state['values']['page_wrappers'] as $key => $wrapper) {
    if (!empty($wrapper['wrapper_nid']) && !isset($allowed_wrappers[$wrapper['wrapper_nid']])) {
      form_set_error('page_wrappers', 'An invalid page wrapper has been selected.');
      $error = TRUE;
    }
  }
  if ($error) {
    $wrappers = _page_wrappers_available_wrappers();
    $filtered_wrappers = springboard_group_page_wrappers_filter($wrappers, $form, $form_state);
    page_wrappers_build_node_form($form, $form_state, $filtered_wrappers);
  }
}

/**
 * Filter a list of wrappers by the node's group membership.
 */
function springboard_group_page_wrappers_filter(&$wrappers, $form, $form_state) {
  $wrappers_are_grouped = og_get_group_type('node', 'page_wrapper', 'group content');
  // If the pagewrapper content type is group enabled,
  // alter the available options.
  if ($wrappers_are_grouped) {

    $trigger = NULL;
    if (!empty($form_state['triggering_element']['#ajax']['callback'])) {
      $trigger = $form_state['triggering_element']['#ajax']['callback'];
    }
    // If a group has been selected limit available wrappers to
    // ones assigned to the selected group.
    // @see springboard_group_ajax_callback().
    if (!empty($form_state['values'])) {
      $gids = [];
      if ($trigger == 'springboard_group_og_group_field_ajax_callback') {
        $gids = springboard_group_get_gids_from_element($form, $form_state);
      }
      elseif ($trigger == '_page_wrappers_add_js') {
        // Multiple pagewrappers feature is
        // enabled and the source of this ajax call.
        $type = $form['#node']->type;
        $language = $form['og_group_ref_' . substr($type, 0, 19)]['#language'];
        $gids = $form_state['input']['og_group_ref_' . substr($type, 0, 19)][$language][0]['default'];
        if (!is_array($gids)) {
          $gids = array($gids);
        }
      }
      $wrappers = springboard_group_page_wrappers_list_wrappers_by_group($gids);

    }
    else {
      // We're on a freshly loaded page.
      $gids = NULL;
      $node = $form['#node'];
      if (!empty($node->nid)) {
        // Check if the node we want to wrap is in a group.
        $node_groups = og_get_entity_groups('node', $node);
        // If the node has a group, set the $gid.
        if (!empty($node_groups['node'])) {
          $gids = $node_groups['node'];
        }
      }
      // Alter the form options.
      $wrappers = springboard_group_page_wrappers_list_wrappers_by_group($gids);
    }
  }
  return $wrappers;
}


/**
 * List page wrappers by group.
 *
 * @param array $gids
 *   Group id.
 *
 * @return array
 *   Associative array of page wrapper titles keyed by node id or FALSE.
 */
function springboard_group_page_wrappers_list_wrappers_by_group($gids) {
  $wrappers = array();
  $query = db_select('node', 'n');
  $query->addField('n', 'nid', 'nid');
  $query->addField('n', 'title', 'title');
  $query->leftJoin('og_membership', 'o', 'o.etid=n.nid');
  $query->condition('n.type', 'page_wrapper');
  if (!empty($gids)) {
    $query->condition('o.gid', $gids);
    $query->groupBy('n.nid');
  }
  else {
    $query->IsNull('o.etid');
  }
  $results = $query->execute();
  while ($record = $results->fetchAssoc()) {
    $wrappers[$record['nid']] = $record['title'];
  }

  return count($wrappers) ? $wrappers : array();
}
