<?php
/**
 * @file
 * Determine what group we are working in based on page or node context.
 */

/**
 * Implements hook_og_context_negotiation_info().
 *
 * Fires off a context negotiator on non-node advocacy paths, so we can figure
 * out what group we're currently working in.
 */
function springboard_group_og_context_negotiation_info() {
  $providers = array();
  $providers['springboard_group'] = array(
    'name' => t('Springboard Groups'),
    'description' => t("Determine context for Springboard Groups"),
    'callback' => 'springboard_group_og_context_handler',
    'menu path' => springboard_group_context_menu_paths(),
  );

  return $providers;
}

/**
 * OG_context handler.
 *
 * On non-node pages, figure out the current group context, or set it.
 */
function springboard_group_og_context_handler($item = NULL) {

  $path = current_path();
  if (!empty($_SESSION['og_context'])) {
    unset($_SESSION['og_context']);
  }
  if (strpos($path, '/edit') !== FALSE && strpos($path, 'advocacy/target' !== FALSE)) {
    //return array('node' => array($gid);
  }
  global $user;
  $ret = array();
  if ($user->uid) {
    if (user_access('administer group')) {
      $groups = springboard_group_get_all_springboard_groups();
    }
    else {
      $groups = og_get_groups_by_user(NULL, 'node');
    }
    if (!empty($groups)) {
      // If the user only belongs to one group, return it.
      if (count($groups) === 1) {
        $gid = reset($groups);
        $_SESSION['springboard_group_context_multiple'] = NULL;
        $ret = array('node' => array($gid));
      }
      // If the user has multiple groups and there is not a currently selected
      // working group, set the first group as the working group, and set
      // springboard_group_context_multiple so that the dashboard knows
      // we need a selector.
      elseif (!isset($_SESSION['springboard_group_working_group'])) {
        $_SESSION['springboard_group_context_multiple'] = array_values($groups);
        $gid = reset($groups);
        $_SESSION['springboard_group_working_group'] = $gid;
        $ret = array('node' => array($gid));
      }
      elseif ($_SESSION['springboard_group_working_group'] == '_none') {
        $_SESSION['springboard_group_context_multiple'] = array_values($groups);
        $ret = array('_none' => array('_none'));
      }
      elseif ($_SESSION['springboard_group_working_group'] == '_all') {
        $_SESSION['springboard_group_working_group'] = '_all';
        $_SESSION['springboard_group_context_multiple'] = array_values($groups);
        $ret = array('_all' => array('_all'));
      }
      else {

        // There's already a working group, return it as the context and
        // tell the dashboard we need a selector.
        $_SESSION['springboard_group_context_multiple'] = array_values($groups);
        $ret = array('node' => array($_SESSION['springboard_group_working_group']));
      }
    }
  }
  return $ret;
}

/**
 * Iterative getter of group IDs.
 *
 * This should only be called from contexts where you are expecting a single
 * group ID to be returned - i.e., non-node pages and nodes on which only
 * a single group selection is allowed (advocacy).
 *
 * @return mixed
 */
function springboard_group_get_group_context($node = NULL) {

  // Note to future: this can not be a static value. It's called from
  // listing pages.
  $gid = NULL;

  // First check if we're on a grouped node or node listing page.
  // BTW, this only happens (so far) in an administrative context,
  // so no big deal performance-wise.
  if (arg(0) == 'node' || !empty($node)) {
    if (empty($node)) {
      $node = menu_get_object();
    }
    if (!empty($node->nid) && !empty($node->vid) && !og_is_group('node', $node->nid)) {
      $wrapper = entity_metadata_wrapper('node', $node);
      foreach ($node as $key => $property) {
        if (strpos($key, 'og_group_ref') !== FALSE) {
          $group = $wrapper->$key->value();
          $gid = !empty($group[0]->nid) ? $group[0]->nid : FALSE;
        }
      }

    }
    // GID can be false if the node is not grouped.
    return $gid;
  }

  // If we've gotten this far, we should be dealing with a pre-defined path
  // only.
  $paths = springboard_group_context_menu_paths();
  $has_path = FALSE;
  foreach ($paths as $path) {
    if (strpos(current_path(), $path) !== FALSE) {
      $has_path = TRUE;
    }
  }
  if (!$has_path) {
    return NULL;
  }

  // This would have been set by
  // springboard_group_og_context_handler().
  $group_context = og_context('node');
  if (!empty($group_context['gid'])) {
    return $group_context['gid'];
  }

  if (module_exists('springboard_group_advocacy') && arg(3) == 'targets' && arg(4) == 'custom-groups' && arg(6) == 'edit') {
    $api_call = springboard_advocacy_api_call();
    $response = $api_call->invokeClientMethod('getTargetGroup', arg(5));
    if (!empty($response->data->affiliate_id)) {
      return $response->data->affiliate_id;
    }
  }

  // "_none", "_all" etc.
  if (!empty($_SESSION['springboard_group_working_group'])) {

    // If the selection is "all" and we're on the "add target" pages,
    // we have to present a dropdown with valid options to the user.
    // This involves removing the "all" option if it is already selected.
    if ($_SESSION['springboard_group_working_group'] == '_all'
      && module_exists('springboard_group_advocacy')
      && ((arg(4) == 'custom-targets' || arg(4) == 'custom-groups') && arg(5) == 'add')
    ) {
      if (user_access('administer group')) {
        return '_none';
      }
      else {
        // It's an affiliate.
        $groups = $_SESSION['springboard_group_context_multiple'];
        $gid = array_shift($groups);
        return $gid;
      }
    }
    return $_SESSION['springboard_group_working_group'];
  }

  // If all else fails, try this.
  if (!empty($_SESSION['og_context']['gid']) && $_SESSION['og_context']['group_type'] == 'node') {
    return $_SESSION['og_context']['gid'];
  }
  return $gid;
}

/**
 * Determines which non-node paths are group enabled.
 *
 * Used by the context handler hook and the groups dropdown selector function
 * to determine which paths the selector should appear on.
 *
 * @return array
 *   An array of paths the context selector will appear on.
 */
function springboard_group_context_menu_paths() {
  $paths = &drupal_static(__FUNCTION__);

  if (!isset($paths)) {
    $groups_enabled_types = springboard_group_get_grouped_types();
    $paths = array();

    if (!empty($groups_enabled_types['node'])) {
      if (isset($groups_enabled_types['rules_config'])) {
        array_push($paths, 'admin/commerce/config/payment-methods');
      }
      if (isset($groups_enabled_types['node']['page_wrapper'])) {
        array_push($paths, 'admin/springboard/asset-library/page_wrapper');
      }
      if (isset($groups_enabled_types['node']['email_wrapper'])) {
        array_push($paths, 'admin/springboard/asset-library/email_wrapper');
      }
      if (isset($groups_enabled_types['node']['page_wrapper']) || isset($groups_enabled_types['node']['email_wrapper'])) {
        array_push($paths, 'admin/springboard/asset-library');
      }
      if (isset($groups_enabled_types['node']['webform_ab'])) {
        array_push($paths, 'admin/content/webform_ab');
      }
      if (isset($groups_enabled_types['node']['springboard_petition'])) {
        array_push($paths, 'admin/springboard/advocacy/petitions/all');
      }
      if (isset($groups_enabled_types['node']['sustainers_upgrade_form'])) {
        array_push($paths, 'admin/springboard/donation-forms/sustainers_upgrade_form');
      }
      foreach ($groups_enabled_types['node'] as $type => $field) {
        if (fundraiser_is_donation_type($type)) {
          array_push($paths, 'admin/springboard');
          array_push($paths, 'admin/springboard/donation-forms/' . $type);
          array_push($paths, 'admin/springboard/donation-forms/all');
          array_push($paths, 'admin/springboard/donation-forms/sustainers_upgrade_form');
        }
        if (in_array($type, webform_variable_get('webform_node_types'))) {
          array_push($paths, 'admin/springboard');
          array_push($paths, 'admin/springboard/forms/all');
          array_push($paths, 'admin/springboard/forms/' . $type);
        }
      }
    }
    $paths = array_merge($paths, module_invoke_all('springboard_group_context_menu_paths'));

  }
  return $paths;
}

/**
 * Get the group context for entity reference field queries.
 *
 * Not currently used, but would be required for p2p autocompletes
 * or other uses of non-OG entity reference fields which reference grouped
 * content.
 *
 * @return array
 *   The current user's groups.
 */
function springboard_group_context_entityreference($form, $form_state) {
  $field = $form['og_group_ref_' . substr($form['#node']->type, 0, 19)];
  $language = $field['#language'];
  if (empty($form_state['values'])) {
    $gids = $field[$language][0]['default']['#default_value'];
  }
  else {
    $gids = springboard_group_get_gids_from_element($form, $form_state);
  }
  $_SESSION['springboard_group_entityreference_context'] = $gids;
  return $gids;
}

/**
 * Called from springboard_group_preprocess_page().
 *
 * Adds a group dropdown selector to designated non-node pages when a user
 * has multiple group affiliations. See springboard_group_context_menu_paths()
 * for a list of paths this is active on.
 */
function springboard_group_context_selector__preprocess_page(&$vars) {
  $path = current_path();
  $og_paths = springboard_group_context_menu_paths();

  // Debug function.
  if (variable_get('springboard_group_debug_context', NULL) && user_access('administer group')) {
    if (arg(0) == 'node' || in_array($path, $og_paths)) {
      $context = springboard_group_get_group_context();
      if (is_numeric($context)) {
        $node = node_load($context);
        $context = check_plain($node->title);
      }
      drupal_set_message(t("Group context: @context", array('@context' => $context)));
    }
  }

  if (in_array($path, $og_paths) && isset($_SESSION['springboard_group_context_multiple'])) {
    $build = array(
      'selector' => drupal_get_form('springboard_group_context_selector_form', $_SESSION['springboard_group_context_multiple']),
    );
    $vars['page']['content']['system_main']['group_selector']['#markup'] = render($build);
    $vars['page']['content']['system_main']['group_selector']['#weight'] = -10;
    $vars['page']['content']['system_main']['group_selector']['#prefix'] = '<div id="sb-og-group-group_selector">';
    $vars['page']['content']['system_main']['group_selector']['#suffix'] = '</div>';
  }
}

/**
 * Build the group selector form.
 *
 * Pages without a node context need a way of determining which group is
 * active. This adds a selector to choose which group
 * context to work in.
 *
 * @param array $groups
 *   The current user's possible groups.
 *
 * @return array
 *   The form.
 */
function springboard_group_context_selector_form($form, &$form_state, $groups) {

  $form = [];
  $form['#attached']['css'][] = drupal_get_path('module', 'springboard_group') . '/css/springboard-group-selector.css';
  $form['title']['#markup'] = '<strong>' . t('Choose a group to work in:') . '</strong>';

  // Build the selector form from groups found in user session.
  $options = [];
  if (user_access('administer group')) {
    $options['_none'] = '- None -';
  }
  if (count($groups) > 1) {
    $options['_all'] = 'All Groups';
  }

  if ((arg(4) == 'custom-groups' || arg(4) == 'custom-targets')
    && arg(5) == 'add' && isset($options['_all'])
  ) {
    unset($options['_all']);
  }

  foreach ($groups as $nid) {
    $node = node_load($nid);
    $options[$nid] = check_plain($node->title);
  }

  // The ajax enabled group selector which will reload the page
  // subsequent to a group choice. The group ID will get put into session.
  $form['groups'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => springboard_group_get_group_context(),
    '#ajax' => array(
      'callback' => 'springboard_group_context_selector_ajax_callback',
      'wrapper' => 'sb-og-group-selector',
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#prefix' => '<div id="group-selector-div">',
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * AJAX callback for group selector form.
 */
function springboard_group_context_selector_ajax_callback($form, &$form_state) {
  // Put the chosen group into session, so it can be retrieved by
  // springboard_group_advocacy_og_context_handler().
  $_SESSION['springboard_group_working_group'] = $form_state['values']['groups'];

  // Reload the page to get the new grouped content.
  ctools_include('ajax');
  ctools_add_js('ajax-responder');
  $commands[] = ctools_ajax_command_reload();
  print ajax_render($commands);
  drupal_exit();
}
