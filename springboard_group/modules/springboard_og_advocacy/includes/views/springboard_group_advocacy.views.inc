<?php
/**
 * @file
 * Views hooks for Springboard Group Advocacy.
 */

/**
 * Implements hook_springboard_advocacy_views_plugin_query_alter().
 *
 * Alter the query plugin so it only returns results for allowed states
 * if the state selector is set to "All" while searching.
 *
 * @todo - this would not be necessary if we were to save the affiliate
 * subscription data on the API server; that is, we could implement very similar
 * logic there in the checkSubscription method (which we do for the
 * "federal-and-states-selected" subscripton level already, out of necessity).
 */
function springboard_group_springboard_advocacy_views_plugin_query_alter(&$query) {
  $gid = springboard_group_get_group_context();
  if (!empty($gid)) {
    if ($gid != '_none' && $gid != '_all') {
      $group_subscription = springboard_group_advocacy_get_saved_group_subscription($gid);
      $type = $group_subscription['group_subscription_type'];
      switch ($type) {
        case 'states-selected':
          if (!empty($query->search_terms) && $query->search_terms['class_name'][0] == 'Legislator') {
            if (empty($query->search_terms['states'])) {
              $states = unserialize($group_subscription['allowed_states']);
              $query->search_terms['state'] = $states;
            }
            if (empty($query->search_terms['role'])) {
              $query->search_terms['role'][] = 'SR|SS|GOVNR';
            }
          }
          break;

        case'federal-and-states-selected':
          if (!empty($query->search_terms) && $query->search_terms['class_name'][0] == 'Legislator') {
            $states = unserialize($group_subscription['allowed_states']);
            $query->search_terms['affiliate_allowed_state'] = $states;
          }
          break;

        case 'state-only':
          if (!empty($query->search_terms) && $query->search_terms['class_name'][0] == 'Legislator') {
            if (empty($query->search_terms['role'])) {
              $query->search_terms['role'][] = 'SR|SS|GOVNR';
            }
          }
          break;

        case 'federal-only':
          if (!empty($query->search_terms) && $query->search_terms['class_name'][0] == 'Legislator') {
            if (empty($query->search_terms['role'])) {
              $query->search_terms['role'][] = 'FR|FS|PRES01|PRES02|PRES03';
            }
          }
          break;
      }
    }
    // If there's a group id, only return results for that group when searching
    // custom targets or custom groups.
    if (!empty($query->search_terms) && ($query->search_terms['class_name'][0] == 'Target' || $query->search_terms['class_name'][0] == 'Group')) {
      if (!user_access('administer_group') && $gid == '_all' && !empty($_SESSION['springboard_group_context_multiple'])) {
        $query->search_terms['affiliate_id'] = $_SESSION['springboard_group_context_multiple'];
      }
      else {
        $query->search_terms['affiliate_id'] = $gid;
      }
    }
  }
  else {
    // There's no group id.
    if (!empty($query->search_terms) && ($query->search_terms['class_name'][0] == 'Target' || $query->search_terms['class_name'][0] == 'Group')) {
      if (!user_access('administer group')) {
        $query->search_terms['affiliate_id'] = '_none';
      }
      else {
        $query->search_terms['affiliate_id'] = '_none';
      }
    }
  }
}


/**
 * Implements hook_views_data_alter().
 *
 * Add data array for Affiliate ID field returned by API query.
 * Call it Affiliate Name because we convert to the group name in
 * springboard_group_preprocess_views_view_table().
 */
function springboard_group_advocacy_views_data_alter(&$data) {
  // Target ID returned by the API request.
  $data['advocacy']['affiliate_id'] = array(
    'title' => t('Affiliate name'),
    'help' => t('Affiliate Name'),
    'field' => array(
      'click sortable' => TRUE,
      'handler' => 'springboard_advocacy_views_handler_field',
    ),
  );
}

/**
 * Implements hook_views_preprocess_views_view_table().
 *
 * Change the Affiliate ID field to display the affiliate name, and make the
 * affiliate name sortable.
 */
function springboard_group_preprocess_views_view_table(&$vars) {
  $view = $vars['view'];
  if ($view->name == 'sba_custom_targets') {
    foreach ($vars['rows'] as $key => $row) {
      foreach ($row as $field_name => $value) {
        if ($field_name == 'affiliate_id') {
          if (is_numeric($value)) {
            $group = node_load($value);
            if (!empty($group->nid) && !empty($group->type) && $group->type == 'springboard_group') {
              $vars['rows'][$key][$field_name] = check_plain($group->title);
            }
            elseif (empty($group->nid) && empty($group->type)) {
              $vars['rows'][$key][$field_name] = t("Group deleted");
            }
          }
        }
      }
    }
  }
  if (isset($_GET['order']) && $_GET['order'] == 'affiliate_id') {
    usort($vars['rows'], function ($a, $b) {
      if ($_GET['sort'] == 'desc') {
        return strcmp($b['affiliate_id'], $a['affiliate_id']);
      }
      else {
        return strcmp($a['affiliate_id'], $b['affiliate_id']);
      }
    });
  }
}

/**
 * Implements hook_views_default_views_alter().
 *
 * Add the Affilaite Group Field to Advocacy dashboard views.
 */
function springboard_group_advocacy_views_default_views_alter(&$views) {
  $displays = $views['sba_custom_targets']->display;
  foreach ($displays as $display_name => $display) {
    $handler =& $views['sba_custom_targets']->display[$display_name]->handler;
    if (!empty($handler->display->display_options['fields'])) {
      $fields =& $handler->display->display_options['fields'];
      $group_field = [];
      $group_field['affiliate_id']['id'] = 'affiliate_id';
      $group_field['affiliate_id']['table'] = 'advocacy';
      $group_field['affiliate_id']['field'] = 'affiliate_id';
      $group_field['affiliate_id']['label'] = 'Owner';
      if ($display_name == 'block_1') {
        springboard_group_views_add_group_field($fields, 5, $group_field);
      }
      else {
        springboard_group_views_add_group_field($fields, 2, $group_field);
      }
    }
  }
}
