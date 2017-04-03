<?php

/**
 * Implements hook_views_query_alter().
 *
 * Fix the bulk add view to display only non-members.
 *
 * Add a join for og_membership table on dashboard views when
 * the content type is in a group.
 */
function springboard_group_views_query_alter(&$view, &$query) {

  if ($view->name == 'springboard_group_bulk_add') {
    foreach ($query->where as $key => &$condition_group) {
      foreach ($condition_group['conditions'] as &$condition) {
        if (strpos($condition['field'], 'og_membership.gid') !== FALSE) {
          // Unset the where clause from the context filter, and
          // grab it's value and add it to a join instead.
          $gid = $condition['value'][':og_membership_gid'];
          unset($query->where[$key]);
          $join = new views_join();
          $join->table = 'og_membership';
          $join->field = 'etid';
          $join->left_table = 'users';
          $join->left_field = 'uid';
          $join->type = 'left';
          $join->extra = array(
            array(
              'field' => 'entity_type',
              'value' => 'user',
            ),
            array(
              'field' => 'gid',
              'value' => $gid,
            ),
          );
          $join->construct();
          $join->adjusted = TRUE;
          $query->add_relationship('og_membership', $join, 'user');
          $condition = db_and();
          $condition->isNull('og_membership.etid');
          $query->add_where(0, $condition);
        }
      }
    }
  }

  // Array of alterable views.
  $alter = variable_get('group_restricted_views', []);

  if (in_array($view->name, $alter)) {
    // First check if the query is selecting a groupable entity. If it isn't,
    // we skip the query alteration.
    //$grouped = springboard_group_views_query_check_content_type($query);

    // Get the current active group ID.
    $context = springboard_group_get_group_context();
    if (empty($context)) {
      // This gets sent to the query as the "gid", causing no result.
      $context = '_you_will_see_nothing';
    }
    // Build the query.
    $condition = db_and();
    if (($context == '_none' || $context == '_all') && (user_access('administer group') || user_access('bypass node access'))) {
      if ($context == '_none') {
        $condition->isNull('og_membership.gid');
      }
      else {
        if ($context == '_all') {
          $all_groups = springboard_group_get_all_springboard_groups();
          $condition->condition('og_membership.gid', $all_groups, 'IN');
          $condition->condition('og_membership.entity_type', 'node');
        }
      }
    }
    else {
      if ($context == '_all') {
        $context = $_SESSION['springboard_group_context_multiple'];
      }
      $condition->condition('og_membership.gid', $context);
      $condition->condition('og_membership.entity_type', 'node');
    }

    // Help views understand what we're doing. Adding the table without
    // the join explicitly defined makes the query messy.
    $def = array(
      'left_table' => 'node',
      'left_field' => 'nid',
      'table' => 'og_membership',
      'field' => 'etid',
    );
    $join = new views_join();
    $join->definition = $def;
    $join->extra = array(
      array(
        'field' => 'entity_type',
        'value' => 'node',
      ),
    );

    $join->construct();
    $join->adjusted = TRUE;

    $query->add_table('og_membership', 'node', $join);
    $query->add_where(0, $condition);

  }
}

/**
 * Check a query for groupable content.
 *
 * @param object $query
 *   The views query.
 *
 * @return bool
 *   Is the view querying groupable content, true or false.
 */
function springboard_group_views_query_check_content_type($query) {
  $is_grouped = FALSE;
  foreach ($query->where as $where) {
    foreach ($where['conditions'] as $condition) {
      if ($condition['field'] == 'node.type') {
        if (!is_array($condition['value'])) {
          $is_grouped = og_get_group_type('node', $condition['value'], 'group content');
        }
        elseif (count($condition['value']) == 1) {
          $is_grouped = og_get_group_type('node', reset($condition['value']), 'group content');
        }
        else {
          if (user_access('administer content')) {
            drupal_set_message('multipe node types in views query', 'error');
          }
          return TRUE;
        }
      }
    }
  }
  return $is_grouped;
}


function springboard_group_views_pre_execute(&$view) {
 // dpq($view->build_info['query']);
}

/**
 * Implements hook_views_default_views_alter().
 *
 * Add a group title field to springboard dashboard views.
 */
function springboard_group_views_default_views_alter(&$views) {

  $alter = array_filter(variable_get('group_restricted_views', []));
  foreach ($alter as $view_name => $value) {
    $displays = $views[$view_name]->display;
    foreach ($displays as $display_name => $display) {
      $handler =& $views[$view_name]->display[$display_name]->handler;
      /* Field: Content: Title */
      if (!empty($handler->display->display_options['fields'])) {
        $handler->display->display_options['relationships']['og_membership_related_node_group']['id'] = 'og_membership_related_node_group';
        $handler->display->display_options['relationships']['og_membership_related_node_group']['table'] = 'og_membership';
        $handler->display->display_options['relationships']['og_membership_related_node_group']['field'] = 'og_membership_related_node_group';
        $fields =& $handler->display->display_options['fields'];

        // Place the field in the appropriate column position per view display.
        $index = 2;
        switch ($view_name) {
          case 'sba_actions';
            $index = 5;
            break;

          case 'sbv_forms';
            switch ($display_name) {
              // Recent forms block.
              case 'block_3':
                $index = 6;
                break;

              // Recent donation forms.
              case 'block_4':
                $index = 5;
                break;

              // All donations.
              case 'block_1':
                $index = 3;
            }
        }

        $group_field = [];
        $group_field['title_sb_group']['id'] = 'title_sb_group';
        $group_field['title_sb_group']['table'] = 'node';
        $group_field['title_sb_group']['field'] = 'title';
        $group_field['title_sb_group']['label'] = 'Group';
        $group_field['title_sb_group']['relationship'] = 'og_membership_related_node_group';
        springboard_group_views_add_group_field($fields, $index, $group_field);
      }
    }
  }
}


/**
 * Inserts the group name display field in a view.
 *
 * @param array $fields
 *   The field array found in $handler->display->display_options['fields'].
 * @param int $position
 *   The point at which to insert after.
 * @param array $group_field
 *   The field to insert.
 */
function springboard_group_views_add_group_field(&$fields, $position, $group_field) {
  $pre = array_slice($fields, 0, $position, TRUE);
  $post = array_slice($fields, $position, NULL, TRUE);
  $fields = array_merge($pre, $group_field, $post);
}

function springboard_group_views_pre_view(&$view, &$display_id, &$args) {
  if ($view->name == 'springboard_group' || $view->name == 'springboard_group_my_groups') {
    if (user_access('administer group') || user_access('create springboard-group content')) {
      drupal_add_css(drupal_get_path('module', 'springboard_group') . '/css/springboard-og-admin.css');
      $content = l(t('Create New group'), 'node/add/springboard-group', array(
        'attributes' => array(
          'class' => array(
            'button',
            'add-button',
          ),
        ),
        'query' => array(
          'destination' => current_path(),
        ),
      ));
      $view->attachment_before = $content;
    }
  }
}

function springboard_group_views_pre_render(&$view) {
  if ($view->name == 'springboard_group_my_groups' && $view->current_display == 'page_1') {
    foreach ($view->result as $index => &$result) {
      if (!user_access('administer group')
        && !og_user_access('node', $result->node_og_membership_nid, 'manage members')) {
        unset($result->node_og_membership_nid);
      }
    }
  }
}