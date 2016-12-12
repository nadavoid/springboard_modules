<?php
/**
 * @file
 * springboard_p2p_personal_campaign.views_default.inc
 */

/**
 * Implements hook_views_default_views().
 */
function springboard_p2p_personal_campaign_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'p2p_personal_campaigns_admin';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Peer to peer Personal campaigns admin';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Personal campaigns';
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['hide_admin_links'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'administer springboard p2p';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'title' => 'title',
    'title_1' => 'title_1',
    'title_2' => 'title_2',
    'field_p2p_campaign_progress' => 'field_p2p_campaign_progress',
    'field_p2p_personal_campaign_goal' => 'field_p2p_personal_campaign_goal',
    'field_p2p_campaign_progress_1' => 'field_p2p_campaign_progress_1',
    'field_p2p_personal_submit_goal' => 'field_p2p_personal_submit_goal',
    'edit_node' => 'edit_node',
    'delete_node' => 'edit_node',
    'clone_node' => 'edit_node',
    'view_node' => 'edit_node',
  );
  $handler->display->display_options['style_options']['default'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['style_options']['info'] = array(
    'title' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'title_1' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'title_2' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_campaign_progress' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_personal_campaign_goal' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_campaign_progress_1' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_personal_submit_goal' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'edit_node' => array(
      'align' => '',
      'separator' => ' | ',
      'empty_column' => 0,
    ),
    'delete_node' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'clone_node' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'view_node' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  $handler->display->display_options['style_options']['sticky'] = TRUE;
  /* Relationship: Entity Reference: Referenced Entity */
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['id'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['table'] = 'field_data_field_p2p_campaign';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['field'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['required'] = TRUE;
  /* Relationship: Entity Reference: Referenced Entity */
  $handler->display->display_options['relationships']['field_p2p_category_target_id']['id'] = 'field_p2p_category_target_id';
  $handler->display->display_options['relationships']['field_p2p_category_target_id']['table'] = 'field_data_field_p2p_category';
  $handler->display->display_options['relationships']['field_p2p_category_target_id']['field'] = 'field_p2p_category_target_id';
  $handler->display->display_options['relationships']['field_p2p_category_target_id']['relationship'] = 'field_p2p_campaign_target_id';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = 'Personal campaign';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title_1']['id'] = 'title_1';
  $handler->display->display_options['fields']['title_1']['table'] = 'node';
  $handler->display->display_options['fields']['title_1']['field'] = 'title';
  $handler->display->display_options['fields']['title_1']['relationship'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['fields']['title_1']['label'] = 'Campaign';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title_2']['id'] = 'title_2';
  $handler->display->display_options['fields']['title_2']['table'] = 'node';
  $handler->display->display_options['fields']['title_2']['field'] = 'title';
  $handler->display->display_options['fields']['title_2']['relationship'] = 'field_p2p_category_target_id';
  $handler->display->display_options['fields']['title_2']['label'] = 'Category';
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['label'] = 'Fundraising Progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_amount';
  /* Field: Content: Fundraising Goal */
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['id'] = 'field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['table'] = 'field_data_field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['field'] = 'field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['settings'] = array(
    'thousand_separator' => ',',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['id'] = 'field_p2p_campaign_progress_1';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['label'] = 'Submissions';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['type'] = 'springboard_p2p_fields_personal_campaign_progress_submissions';
  /* Field: Content: Submission Goal */
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['id'] = 'field_p2p_personal_submit_goal';
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['table'] = 'field_data_field_p2p_personal_submit_goal';
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['field'] = 'field_p2p_personal_submit_goal';
  /* Field: Content: Edit link */
  $handler->display->display_options['fields']['edit_node']['id'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['table'] = 'views_entity_node';
  $handler->display->display_options['fields']['edit_node']['field'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['label'] = 'Action';
  /* Field: Content: Clone link */
  $handler->display->display_options['fields']['clone_node']['id'] = 'clone_node';
  $handler->display->display_options['fields']['clone_node']['table'] = 'node';
  $handler->display->display_options['fields']['clone_node']['field'] = 'clone_node';
  /* Field: Content: Link */
  $handler->display->display_options['fields']['view_node']['id'] = 'view_node';
  $handler->display->display_options['fields']['view_node']['table'] = 'views_entity_node';
  $handler->display->display_options['fields']['view_node']['field'] = 'view_node';
  /* Sort criterion: Content: Progress (field_p2p_campaign_progress:amount) */
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['id'] = 'field_p2p_campaign_progress_amount';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['field'] = 'field_p2p_campaign_progress_amount';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['order'] = 'DESC';
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );
  /* Filter criterion: Global: Combine fields filter */
  $handler->display->display_options['filters']['combine']['id'] = 'combine';
  $handler->display->display_options['filters']['combine']['table'] = 'views';
  $handler->display->display_options['filters']['combine']['field'] = 'combine';
  $handler->display->display_options['filters']['combine']['operator'] = 'contains';
  $handler->display->display_options['filters']['combine']['exposed'] = TRUE;
  $handler->display->display_options['filters']['combine']['expose']['operator_id'] = 'combine_op';
  $handler->display->display_options['filters']['combine']['expose']['label'] = 'Search';
  $handler->display->display_options['filters']['combine']['expose']['operator'] = 'combine_op';
  $handler->display->display_options['filters']['combine']['expose']['identifier'] = 'combine';
  $handler->display->display_options['filters']['combine']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
    6 => 0,
  );
  $handler->display->display_options['filters']['combine']['fields'] = array(
    'title' => 'title',
    'title_1' => 'title_1',
    'title_2' => 'title_2',
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Bulk operations: Content */
  $handler->display->display_options['fields']['views_bulk_operations']['id'] = 'views_bulk_operations';
  $handler->display->display_options['fields']['views_bulk_operations']['table'] = 'node';
  $handler->display->display_options['fields']['views_bulk_operations']['field'] = 'views_bulk_operations';
  $handler->display->display_options['fields']['views_bulk_operations']['vbo_settings']['display_type'] = '0';
  $handler->display->display_options['fields']['views_bulk_operations']['vbo_settings']['enable_select_all_pages'] = 1;
  $handler->display->display_options['fields']['views_bulk_operations']['vbo_settings']['force_single'] = 0;
  $handler->display->display_options['fields']['views_bulk_operations']['vbo_settings']['entity_load_capacity'] = '10';
  $handler->display->display_options['fields']['views_bulk_operations']['vbo_operations'] = array(
    'action::node_assign_owner_action' => array(
      'selected' => 1,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::views_bulk_operations_delete_item' => array(
      'selected' => 1,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::views_bulk_operations_script_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_make_sticky_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_make_unsticky_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::views_bulk_operations_modify_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
      'settings' => array(
        'show_all_tokens' => 1,
        'display_values' => array(
          '_all_' => '_all_',
        ),
      ),
    ),
    'action::views_bulk_operations_argument_selector_action' => array(
      'selected' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
      'settings' => array(
        'url' => '',
      ),
    ),
    'action::node_promote_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_publish_action' => array(
      'selected' => 1,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_unpromote_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_save_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::system_send_email_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::panelizer_set_status_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::_fundraiser_gateway_switch_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_unpublish_action' => array(
      'selected' => 1,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::node_unpublish_by_keyword_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
    'action::pathauto_node_update_action' => array(
      'selected' => 0,
      'postpone_processing' => 0,
      'skip_confirmation' => 0,
      'override_label' => 0,
      'label' => '',
    ),
  );
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = 'Personal campaign';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title_1']['id'] = 'title_1';
  $handler->display->display_options['fields']['title_1']['table'] = 'node';
  $handler->display->display_options['fields']['title_1']['field'] = 'title';
  $handler->display->display_options['fields']['title_1']['relationship'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['fields']['title_1']['label'] = 'Campaign';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title_2']['id'] = 'title_2';
  $handler->display->display_options['fields']['title_2']['table'] = 'node';
  $handler->display->display_options['fields']['title_2']['field'] = 'title';
  $handler->display->display_options['fields']['title_2']['relationship'] = 'field_p2p_category_target_id';
  $handler->display->display_options['fields']['title_2']['label'] = 'Category';
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['label'] = 'Fundraising Progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_amount';
  /* Field: Content: Fundraising Goal */
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['id'] = 'field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['table'] = 'field_data_field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['field'] = 'field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['settings'] = array(
    'thousand_separator' => ',',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['id'] = 'field_p2p_campaign_progress_1';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['label'] = 'Submissions';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress_1']['type'] = 'springboard_p2p_fields_personal_campaign_progress_submissions';
  /* Field: Content: Submission Goal */
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['id'] = 'field_p2p_personal_submit_goal';
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['table'] = 'field_data_field_p2p_personal_submit_goal';
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['field'] = 'field_p2p_personal_submit_goal';
  /* Field: Content: Edit link */
  $handler->display->display_options['fields']['edit_node']['id'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['table'] = 'views_entity_node';
  $handler->display->display_options['fields']['edit_node']['field'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['label'] = 'Action';
  /* Field: Content: Clone link */
  $handler->display->display_options['fields']['clone_node']['id'] = 'clone_node';
  $handler->display->display_options['fields']['clone_node']['table'] = 'node';
  $handler->display->display_options['fields']['clone_node']['field'] = 'clone_node';
  /* Field: Content: Link */
  $handler->display->display_options['fields']['view_node']['id'] = 'view_node';
  $handler->display->display_options['fields']['view_node']['table'] = 'views_entity_node';
  $handler->display->display_options['fields']['view_node']['field'] = 'view_node';
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );
  /* Filter criterion: Global: Combine fields filter */
  $handler->display->display_options['filters']['combine']['id'] = 'combine';
  $handler->display->display_options['filters']['combine']['table'] = 'views';
  $handler->display->display_options['filters']['combine']['field'] = 'combine';
  $handler->display->display_options['filters']['combine']['operator'] = 'contains';
  $handler->display->display_options['filters']['combine']['exposed'] = TRUE;
  $handler->display->display_options['filters']['combine']['expose']['operator_id'] = 'combine_op';
  $handler->display->display_options['filters']['combine']['expose']['label'] = 'Search';
  $handler->display->display_options['filters']['combine']['expose']['operator'] = 'combine_op';
  $handler->display->display_options['filters']['combine']['expose']['identifier'] = 'combine';
  $handler->display->display_options['filters']['combine']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
    6 => 0,
  );
  $handler->display->display_options['filters']['combine']['fields'] = array(
    'title' => 'title',
    'title_1' => 'title_1',
    'title_2' => 'title_2',
  );
  /* Filter criterion: Date: Date (node) */
  $handler->display->display_options['filters']['date_filter']['id'] = 'date_filter';
  $handler->display->display_options['filters']['date_filter']['table'] = 'node';
  $handler->display->display_options['filters']['date_filter']['field'] = 'date_filter';
  $handler->display->display_options['filters']['date_filter']['operator'] = 'between';
  $handler->display->display_options['filters']['date_filter']['exposed'] = TRUE;
  $handler->display->display_options['filters']['date_filter']['expose']['operator_id'] = 'date_filter_op';
  $handler->display->display_options['filters']['date_filter']['expose']['operator'] = 'date_filter_op';
  $handler->display->display_options['filters']['date_filter']['expose']['identifier'] = 'date_filter';
  $handler->display->display_options['filters']['date_filter']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
    6 => 0,
  );
  $handler->display->display_options['filters']['date_filter']['form_type'] = 'date_popup';
  $handler->display->display_options['filters']['date_filter']['date_fields'] = array(
    'node.created' => 'node.created',
  );
  $handler->display->display_options['path'] = 'admin/springboard/p2p/personal-campaigns';

  /* Display: Block */
  $handler = $view->new_display('block', 'Block', 'block_1');
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );
  $export['p2p_personal_campaigns_admin'] = $view;

  $view = new view();
  $view->name = 'p2p_personal_campaigns_by_campaign';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Peer to peer Personal campaigns by campaign';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Personal campaigns by campaign';
  $handler->display->display_options['hide_admin_links'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['row_plugin'] = 'node';
  $handler->display->display_options['row_options']['links'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Contextual filter: Content: Campaign (field_p2p_campaign) */
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['id'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['table'] = 'field_data_field_p2p_campaign';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['field'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['specify_validation'] = TRUE;
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['validate']['type'] = 'node';
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['validate_options']['types'] = array(
    'p2p_campaign' => 'p2p_campaign',
  );
  $handler->display->display_options['arguments']['field_p2p_campaign_target_id']['validate_options']['access'] = TRUE;
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'p2p/campaigns/campaign';
  $export['p2p_personal_campaigns_by_campaign'] = $view;

  $view = new view();
  $view->name = 'p2p_personal_campaigns_by_campaigner';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Peer to peer Personal campaigns by campaigner';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Personal campaigns by campaigner';
  $handler->display->display_options['hide_admin_links'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['group_by'] = TRUE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'title' => 'title',
    'field_p2p_personal_submit_goal' => 'field_p2p_personal_submit_goal',
    'field_p2p_personal_campaign_goal' => 'field_p2p_personal_campaign_goal',
    'field_p2p_campaign_progress' => 'field_p2p_campaign_progress',
    'field_p2p_fundraising_deadline' => 'field_p2p_fundraising_deadline',
    'edit_node' => 'nid',
    'nid' => 'nid',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'title' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_personal_submit_goal' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_personal_campaign_goal' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_campaign_progress' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_fundraising_deadline' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'edit_node' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'nid' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => ' | ',
      'empty_column' => 0,
    ),
  );
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = 'No personal campaigns found.';
  $handler->display->display_options['empty']['area']['format'] = 'filtered_html';
  /* Relationship: P2P action: Campaign form */
  $handler->display->display_options['relationships']['form_nid']['id'] = 'form_nid';
  $handler->display->display_options['relationships']['form_nid']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['relationships']['form_nid']['field'] = 'form_nid';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Field: Content: Submission Goal */
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['id'] = 'field_p2p_personal_submit_goal';
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['table'] = 'field_data_field_p2p_personal_submit_goal';
  $handler->display->display_options['fields']['field_p2p_personal_submit_goal']['field'] = 'field_p2p_personal_submit_goal';
  /* Field: Content: Fundraising Goal */
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['id'] = 'field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['table'] = 'field_data_field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['field'] = 'field_p2p_personal_campaign_goal';
  $handler->display->display_options['fields']['field_p2p_personal_campaign_goal']['settings'] = array(
    'thousand_separator' => ' ',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_auto';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['group_column'] = 'amount';
  /* Field: Content: Campaign Deadline */
  $handler->display->display_options['fields']['field_p2p_fundraising_deadline']['id'] = 'field_p2p_fundraising_deadline';
  $handler->display->display_options['fields']['field_p2p_fundraising_deadline']['table'] = 'field_data_field_p2p_fundraising_deadline';
  $handler->display->display_options['fields']['field_p2p_fundraising_deadline']['field'] = 'field_p2p_fundraising_deadline';
  $handler->display->display_options['fields']['field_p2p_fundraising_deadline']['label'] = 'Deadline';
  $handler->display->display_options['fields']['field_p2p_fundraising_deadline']['settings'] = array(
    'format_type' => 'long',
    'fromto' => 'both',
    'multiple_number' => '',
    'multiple_from' => '',
    'multiple_to' => '',
  );
  /* Field: Content: Edit link */
  $handler->display->display_options['fields']['edit_node']['id'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['table'] = 'views_entity_node';
  $handler->display->display_options['fields']['edit_node']['field'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['label'] = '';
  $handler->display->display_options['fields']['edit_node']['element_label_colon'] = FALSE;
  /* Field: Content: Nid */
  $handler->display->display_options['fields']['nid_1']['id'] = 'nid_1';
  $handler->display->display_options['fields']['nid_1']['table'] = 'node';
  $handler->display->display_options['fields']['nid_1']['field'] = 'nid';
  $handler->display->display_options['fields']['nid_1']['label'] = 'personal campaign nid';
  $handler->display->display_options['fields']['nid_1']['exclude'] = TRUE;
  /* Field: Content: Nid */
  $handler->display->display_options['fields']['nid']['id'] = 'nid';
  $handler->display->display_options['fields']['nid']['table'] = 'node';
  $handler->display->display_options['fields']['nid']['field'] = 'nid';
  $handler->display->display_options['fields']['nid']['relationship'] = 'form_nid';
  $handler->display->display_options['fields']['nid']['label'] = 'Actions';
  $handler->display->display_options['fields']['nid']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['nid']['alter']['text'] = '<a href="/user/!1/personal-campaigns/campaign-donors/[nid]/[nid_1]">Donors</a>';
  $handler->display->display_options['fields']['nid']['alter']['alt'] = 'Donors';
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Contextual filter: Content: Author uid */
  $handler->display->display_options['arguments']['uid']['id'] = 'uid';
  $handler->display->display_options['arguments']['uid']['table'] = 'node';
  $handler->display->display_options['arguments']['uid']['field'] = 'uid';
  $handler->display->display_options['arguments']['uid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['uid']['default_argument_type'] = 'current_user';
  $handler->display->display_options['arguments']['uid']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['uid']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['uid']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['uid']['specify_validation'] = TRUE;
  $handler->display->display_options['arguments']['uid']['validate']['type'] = 'user';
  $handler->display->display_options['arguments']['uid']['validate']['fail'] = 'access denied';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'p2p/campaigns/user';
  $export['p2p_personal_campaigns_by_campaigner'] = $view;

  $view = new view();
  $view->name = 'p2p_personal_campaigns_by_category';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = ' Peer to peer Personal campaigns by category';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Personal campaigns by category';
  $handler->display->display_options['hide_admin_links'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['row_plugin'] = 'node';
  $handler->display->display_options['row_options']['links'] = FALSE;
  /* Relationship: Entity Reference: Referenced Entity */
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['id'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['table'] = 'field_data_field_p2p_campaign';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['field'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['required'] = TRUE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Contextual filter: Content: Category (field_p2p_category) */
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['id'] = 'field_p2p_category_target_id';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['table'] = 'field_data_field_p2p_category';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['field'] = 'field_p2p_category_target_id';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['relationship'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['specify_validation'] = TRUE;
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['validate']['type'] = 'node';
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['validate_options']['types'] = array(
    'p2p_category' => 'p2p_category',
  );
  $handler->display->display_options['arguments']['field_p2p_category_target_id']['validate_options']['access'] = TRUE;
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'p2p/campaigns/category';
  $export['p2p_personal_campaigns_by_category'] = $view;

  $view = new view();
  $view->name = 'p2p_suggested_donation';
  $view->description = 'The suggested donation for a user on a p2p personal campaign page';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'P2P Suggested Donation';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['style_options']['default_row_class'] = FALSE;
  $handler->display->display_options['style_options']['row_class_special'] = FALSE;
  $handler->display->display_options['row_plugin'] = 'fields';
  /* Relationship: Content: Author */
  $handler->display->display_options['relationships']['uid']['id'] = 'uid';
  $handler->display->display_options['relationships']['uid']['table'] = 'node';
  $handler->display->display_options['relationships']['uid']['field'] = 'uid';
  /* Relationship: Entity Reference: Referenced Entity */
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['id'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['table'] = 'field_data_field_p2p_campaign';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['field'] = 'field_p2p_campaign_target_id';
  /* Field: User: First name */
  $handler->display->display_options['fields']['sbp_first_name']['id'] = 'sbp_first_name';
  $handler->display->display_options['fields']['sbp_first_name']['table'] = 'field_data_sbp_first_name';
  $handler->display->display_options['fields']['sbp_first_name']['field'] = 'sbp_first_name';
  $handler->display->display_options['fields']['sbp_first_name']['relationship'] = 'uid';
  $handler->display->display_options['fields']['sbp_first_name']['label'] = '';
  $handler->display->display_options['fields']['sbp_first_name']['exclude'] = TRUE;
  $handler->display->display_options['fields']['sbp_first_name']['element_label_colon'] = FALSE;
  /* Field: Content: Suggested Donation Amount */
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['id'] = 'field_p2p_suggested_donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['table'] = 'field_data_field_p2p_suggested_donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['field'] = 'field_p2p_suggested_donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['relationship'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['label'] = '';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['alter']['text'] = '[sbp_first_name] suggests a donation of [field_p2p_suggested_donation]';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_label_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['settings'] = array(
    'thousand_separator' => ' ',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  /* Field: Content: Suggested Donation Amount */
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['id'] = 'field_p2p_suggested_donation_amt';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['table'] = 'field_data_field_p2p_suggested_donation_amt';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['field'] = 'field_p2p_suggested_donation_amt';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['label'] = '';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['alter']['text'] = '[sbp_first_name] suggests a donation of [field_p2p_suggested_donation_amt]';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_label_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['settings'] = array(
    'thousand_separator' => ' ',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Contextual filter: Content: Nid */
  $handler->display->display_options['arguments']['nid']['id'] = 'nid';
  $handler->display->display_options['arguments']['nid']['table'] = 'node';
  $handler->display->display_options['arguments']['nid']['field'] = 'nid';
  $handler->display->display_options['arguments']['nid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['nid']['default_argument_type'] = 'node';
  $handler->display->display_options['arguments']['nid']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['nid']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['nid']['summary_options']['items_per_page'] = '25';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;

  /* Display: P2P Personal Suggested */
  $handler = $view->new_display('panel_pane', 'P2P Personal Suggested', 'panel_pane_1');
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Suggested Donation Amount */
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['id'] = 'field_p2p_suggested_donation_amt';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['table'] = 'field_data_field_p2p_suggested_donation_amt';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['field'] = 'field_p2p_suggested_donation_amt';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['label'] = 'Suggested Donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_label_type'] = 'div';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_label_class'] = 'field-label';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_wrapper_type'] = 'div';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_wrapper_class'] = 'field';
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['hide_empty'] = TRUE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation_amt']['settings'] = array(
    'thousand_separator' => ',',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  $handler->display->display_options['pane_category']['name'] = 'Springboard P2P';
  $handler->display->display_options['pane_category']['weight'] = '0';

  /* Display: P2P Campaign Suggested */
  $handler = $view->new_display('panel_pane', 'P2P Campaign Suggested', 'panel_pane_2');
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Suggested Donation Amount */
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['id'] = 'field_p2p_suggested_donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['table'] = 'field_data_field_p2p_suggested_donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['field'] = 'field_p2p_suggested_donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['relationship'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['label'] = 'Suggested Donation';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_label_type'] = 'div';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_label_class'] = 'field-label';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_wrapper_type'] = 'div';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_wrapper_class'] = 'field';
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['hide_empty'] = TRUE;
  $handler->display->display_options['fields']['field_p2p_suggested_donation']['settings'] = array(
    'thousand_separator' => ',',
    'decimal_separator' => '.',
    'scale' => '2',
    'prefix_suffix' => 1,
  );
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  $handler->display->display_options['pane_category']['name'] = 'Springboard P2P';
  $handler->display->display_options['pane_category']['weight'] = '0';
  $export['p2p_suggested_donation'] = $view;

  $view = new view();
  $view->name = 'p2p_top_personal_campaigns_by_category';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Peer to peer Top Personal campaigns by category';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Top Personal Campaigns';
  $handler->display->display_options['hide_admin_links'] = TRUE;
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'title' => 'title',
    'field_p2p_campaign_progress' => 'field_p2p_campaign_progress',
    'progress' => 'progress',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'title' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_p2p_campaign_progress' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'progress' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* Relationship: Entity Reference: Referenced Entity */
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['id'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['table'] = 'field_data_field_p2p_campaign';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['field'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['relationships']['field_p2p_campaign_target_id']['label'] = 'A bridge to the Content entity that is referenced via field_p2p_campaign';
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = 'Campaigns';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_auto';
  /* Sort criterion: Content: Progress (field_p2p_campaign_progress:amount) */
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['id'] = 'field_p2p_campaign_progress_amount';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['field'] = 'field_p2p_campaign_progress_amount';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['order'] = 'DESC';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_amount']['expose']['label'] = 'Progress (field_p2p_campaign_progress:amount)';
  /* Sort criterion: Content: Progress (field_p2p_campaign_progress:submissions) */
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_submissions']['id'] = 'field_p2p_campaign_progress_submissions';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_submissions']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_submissions']['field'] = 'field_p2p_campaign_progress_submissions';
  $handler->display->display_options['sorts']['field_p2p_campaign_progress_submissions']['order'] = 'DESC';
  /* Contextual filter: Content: Nid */
  $handler->display->display_options['arguments']['nid']['id'] = 'nid';
  $handler->display->display_options['arguments']['nid']['table'] = 'node';
  $handler->display->display_options['arguments']['nid']['field'] = 'nid';
  $handler->display->display_options['arguments']['nid']['relationship'] = 'field_p2p_campaign_target_id';
  $handler->display->display_options['arguments']['nid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['nid']['default_argument_type'] = 'node';
  $handler->display->display_options['arguments']['nid']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['nid']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['nid']['summary_options']['items_per_page'] = '25';
  /* Filter criterion: Content: Published */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'node';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = 1;
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;
  /* Filter criterion: Content: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'node';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'p2p_personal_campaign' => 'p2p_personal_campaign',
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_auto';
  $handler->display->display_options['path'] = 'p2p/campaigns/top';

  /* Display: Campaign contextual pane */
  $handler = $view->new_display('panel_pane', 'Campaign contextual pane', 'panel_pane_1');
  $handler->display->display_options['defaults']['css_class'] = FALSE;
  $handler->display->display_options['css_class'] = 'top-personal-campaigns-view';
  $handler->display->display_options['defaults']['use_ajax'] = FALSE;
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'mini';
  $handler->display->display_options['pager']['options']['items_per_page'] = '4';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pane_category']['name'] = 'Springboard P2P';
  $handler->display->display_options['pane_category']['weight'] = '0';

  /* Display: Campaign contextual Attachment */
  $handler = $view->new_display('attachment', 'Campaign contextual Attachment', 'attachment_1');
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'list';
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['row_plugin'] = 'fields';
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_type'] = 'h3';
  $handler->display->display_options['fields']['title']['element_class'] = 'highest-campaign';
  $handler->display->display_options['fields']['title']['element_label_type'] = '0';
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['title']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['title']['element_default_classes'] = FALSE;
  /* Field: Content: Upload a Campaign Image */
  $handler->display->display_options['fields']['field_p2p_campaign_images']['id'] = 'field_p2p_campaign_images';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['table'] = 'field_data_field_p2p_campaign_images';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['field'] = 'field_p2p_campaign_images';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['label'] = '';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_class'] = 'personal-campaign-image';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_label_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['settings'] = array(
    'image_style' => 'p2p_campaign_image_landing',
    'image_link' => '',
  );
  $handler->display->display_options['fields']['field_p2p_campaign_images']['delta_limit'] = '1';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['delta_offset'] = '0';
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_label_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_wrapper_type'] = 'h4';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_wrapper_class'] = 'highest-progress-title';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_auto';
  $handler->display->display_options['displays'] = array(
    'panel_pane_1' => 'panel_pane_1',
    'default' => 0,
    'page' => 0,
  );

  /* Display: Landing pane */
  $handler = $view->new_display('panel_pane', 'Landing pane', 'panel_pane_2');
  $handler->display->display_options['defaults']['css_class'] = FALSE;
  $handler->display->display_options['css_class'] = 'top-personal-campaigns-view';
  $handler->display->display_options['defaults']['use_ajax'] = FALSE;
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'mini';
  $handler->display->display_options['pager']['options']['items_per_page'] = '4';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  $handler->display->display_options['pane_category']['name'] = 'Springboard P2P';
  $handler->display->display_options['pane_category']['weight'] = '0';

  /* Display: Landing Attachment */
  $handler = $view->new_display('attachment', 'Landing Attachment', 'attachment_2');
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '1';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'list';
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['row_plugin'] = 'fields';
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = '';
  $handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
  $handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
  $handler->display->display_options['fields']['title']['element_type'] = 'h3';
  $handler->display->display_options['fields']['title']['element_class'] = 'highest-campaign';
  $handler->display->display_options['fields']['title']['element_label_type'] = '0';
  $handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['title']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['title']['element_default_classes'] = FALSE;
  /* Field: Content: Upload a Campaign Image */
  $handler->display->display_options['fields']['field_p2p_campaign_images']['id'] = 'field_p2p_campaign_images';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['table'] = 'field_data_field_p2p_campaign_images';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['field'] = 'field_p2p_campaign_images';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['label'] = '';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_class'] = 'personal-campaign-image';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_label_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_campaign_images']['element_wrapper_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['click_sort_column'] = 'fid';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['settings'] = array(
    'image_style' => 'p2p_campaign_image_landing',
    'image_link' => '',
  );
  $handler->display->display_options['fields']['field_p2p_campaign_images']['delta_limit'] = '1';
  $handler->display->display_options['fields']['field_p2p_campaign_images']['delta_offset'] = '0';
  /* Field: Content: Progress */
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['id'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['table'] = 'field_data_field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['field'] = 'field_p2p_campaign_progress';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_label_type'] = '0';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_wrapper_type'] = 'h4';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_wrapper_class'] = 'highest-progress-title';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['click_sort_column'] = 'amount';
  $handler->display->display_options['fields']['field_p2p_campaign_progress']['type'] = 'springboard_p2p_fields_personal_campaign_progress_auto';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  $handler->display->display_options['displays'] = array(
    'panel_pane_2' => 'panel_pane_2',
    'default' => 0,
    'page' => 0,
    'panel_pane_1' => 0,
  );
  $export['p2p_top_personal_campaigns_by_category'] = $view;

  $view = new view();
  $view->name = 'personal_campaign_donor_dashboard';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'node';
  $view->human_name = 'Personal Campaign Donor Dashboard';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  
  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'donor_name' => 'donor_name',
    'amount' => 'amount',
    'created' => 'created',
    'mail' => 'mail',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'donor_name' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'amount' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'created' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'mail' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['label'] = 'No results found';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = 'We got nothing here.';
  $handler->display->display_options['empty']['area']['format'] = 'filtered_html';
  /* Relationship: P2P action: User. */
  $handler->display->display_options['relationships']['uid']['id'] = 'uid';
  $handler->display->display_options['relationships']['uid']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['relationships']['uid']['field'] = 'uid';
  $handler->display->display_options['relationships']['uid']['label'] = 'Personal Campaign action user';
  /* Relationship: P2P action: Campaign form */
  $handler->display->display_options['relationships']['form_nid']['id'] = 'form_nid';
  $handler->display->display_options['relationships']['form_nid']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['relationships']['form_nid']['field'] = 'form_nid';
  /* Relationship: P2P action: P2P personal campaign */
  $handler->display->display_options['relationships']['personal_campaign_nid']['id'] = 'personal_campaign_nid';
  $handler->display->display_options['relationships']['personal_campaign_nid']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['relationships']['personal_campaign_nid']['field'] = 'personal_campaign_nid';
  /* Field: P2P action: User Name */
  $handler->display->display_options['fields']['donor_name']['id'] = 'donor_name';
  $handler->display->display_options['fields']['donor_name']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['fields']['donor_name']['field'] = 'donor_name';
  $handler->display->display_options['fields']['donor_name']['label'] = 'Name';
  /* Field: P2P action: Action amount */
  $handler->display->display_options['fields']['amount']['id'] = 'amount';
  $handler->display->display_options['fields']['amount']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['fields']['amount']['field'] = 'amount';
  /* Field: P2P action: Date */
  $handler->display->display_options['fields']['created']['id'] = 'created';
  $handler->display->display_options['fields']['created']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['fields']['created']['field'] = 'created';
  $handler->display->display_options['fields']['created']['date_format'] = 'custom';
  $handler->display->display_options['fields']['created']['custom_date_format'] = 'm/d/Y';
  /* Field: User: E-mail */
  $handler->display->display_options['fields']['mail']['id'] = 'mail';
  $handler->display->display_options['fields']['mail']['table'] = 'users';
  $handler->display->display_options['fields']['mail']['field'] = 'mail';
  $handler->display->display_options['fields']['mail']['relationship'] = 'uid';
  $handler->display->display_options['fields']['mail']['alter']['text'] = '<a href="mailto:[mail]">email</a>';
  /* Sort criterion: Content: Post date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'node';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Contextual filter: Content: Nid */
  $handler->display->display_options['arguments']['nid']['id'] = 'nid';
  $handler->display->display_options['arguments']['nid']['table'] = 'node';
  $handler->display->display_options['arguments']['nid']['field'] = 'nid';
  $handler->display->display_options['arguments']['nid']['relationship'] = 'form_nid';
  $handler->display->display_options['arguments']['nid']['default_action'] = 'empty';
  $handler->display->display_options['arguments']['nid']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['nid']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['nid']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['nid']['summary_options']['items_per_page'] = '25';
  /* Contextual filter: Content: Nid */
  $handler->display->display_options['arguments']['nid_1']['id'] = 'nid_1';
  $handler->display->display_options['arguments']['nid_1']['table'] = 'node';
  $handler->display->display_options['arguments']['nid_1']['field'] = 'nid';
  $handler->display->display_options['arguments']['nid_1']['relationship'] = 'personal_campaign_nid';
  $handler->display->display_options['arguments']['nid_1']['default_action'] = 'empty';
  $handler->display->display_options['arguments']['nid_1']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['nid_1']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['nid_1']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['nid_1']['summary_options']['items_per_page'] = '25';
  /* Filter criterion: P2P action: Status */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'springboard_p2p_personal_campaign_action';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = '1';
  
  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->display->display_options['path'] = 'p2p_personal_campaign_donor_dashboard';
  $export['personal_campaign_donor_dashboard'] = $view;

  return $export;
}
