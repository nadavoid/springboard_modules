<?php
/**
 * @file
 * Provides default view configurations.
 */

/**
 * Implements hook_views_default_views().
 */
function fundraiser_designations_views_default_views() {

  $view = new view();
  $view->name = 'fundraiser_designations';
  $view->description = '';
  $view->tag = '';
  $view->base_table = 'commerce_product';
  $view->human_name = 'Fundraiser Designations';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Fundraiser Designations';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'create commerce_product entities of bundle fundraiser_designation';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['submit_button'] = 'Filter';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '50';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'product_id' => 'product_id',
    'title' => 'title',
    'field_fd_designation_categories' => 'field_fd_designation_categories',
    'delete_product' => 'edit_product',
    'edit_product' => 'edit_product',
  );
  $handler->display->display_options['style_options']['default'] = 'product_id';
  $handler->display->display_options['style_options']['info'] = array(
    'product_id' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'title' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'field_fd_designation_categories' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'delete_product' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'edit_product' => array(
      'align' => '',
      'separator' => ' | ',
      'empty_column' => 0,
    ),
  );
  /* Field: Commerce Product: Product ID */
  $handler->display->display_options['fields']['product_id']['id'] = 'product_id';
  $handler->display->display_options['fields']['product_id']['table'] = 'commerce_product';
  $handler->display->display_options['fields']['product_id']['field'] = 'product_id';
  $handler->display->display_options['fields']['product_id']['exclude'] = TRUE;
  $handler->display->display_options['fields']['product_id']['link_to_product'] = 0;
  /* Field: Commerce Product: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'commerce_product';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['link_to_product'] = 0;
  /* Field: Commerce Product: Fund Categories */
  $handler->display->display_options['fields']['field_fd_designation_categories']['id'] = 'field_fd_designation_categories';
  $handler->display->display_options['fields']['field_fd_designation_categories']['table'] = 'field_data_field_fd_designation_categories';
  $handler->display->display_options['fields']['field_fd_designation_categories']['field'] = 'field_fd_designation_categories';
  $handler->display->display_options['fields']['field_fd_designation_categories']['label'] = 'Fund Category';
  $handler->display->display_options['fields']['field_fd_designation_categories']['type'] = 'taxonomy_term_reference_plain';
  $handler->display->display_options['fields']['field_fd_designation_categories']['delta_offset'] = '0';
  /* Field: Commerce Product: Delete link */
  $handler->display->display_options['fields']['delete_product']['id'] = 'delete_product';
  $handler->display->display_options['fields']['delete_product']['table'] = 'commerce_product';
  $handler->display->display_options['fields']['delete_product']['field'] = 'delete_product';
  /* Field: Commerce Product: Edit link */
  $handler->display->display_options['fields']['edit_product']['id'] = 'edit_product';
  $handler->display->display_options['fields']['edit_product']['table'] = 'commerce_product';
  $handler->display->display_options['fields']['edit_product']['field'] = 'edit_product';
  $handler->display->display_options['fields']['edit_product']['label'] = 'Actions';
  /* Filter criterion: Commerce Product: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'commerce_product';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'fundraiser_designation' => 'fundraiser_designation',
  );
  /* Filter criterion: Commerce Product: Fund Categories (field_fd_designation_categories) */
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['id'] = 'field_fd_designation_categories_tid';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['table'] = 'field_data_field_fd_designation_categories';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['field'] = 'field_fd_designation_categories_tid';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['exposed'] = TRUE;
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['expose']['operator_id'] = 'field_fd_designation_categories_tid_op';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['expose']['label'] = 'Category';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['expose']['operator'] = 'field_fd_designation_categories_tid_op';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['expose']['identifier'] = 'field_fd_designation_categories_tid';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['type'] = 'select';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['vocabulary'] = 'fd_designation_categories';
  $handler->display->display_options['filters']['field_fd_designation_categories_tid']['hierarchy'] = 1;
  /* Filter criterion: Commerce Product: Title */
  $handler->display->display_options['filters']['title']['id'] = 'title';
  $handler->display->display_options['filters']['title']['table'] = 'commerce_product';
  $handler->display->display_options['filters']['title']['field'] = 'title';
  $handler->display->display_options['filters']['title']['operator'] = 'contains';
  $handler->display->display_options['filters']['title']['exposed'] = TRUE;
  $handler->display->display_options['filters']['title']['expose']['operator_id'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['label'] = 'Title';
  $handler->display->display_options['filters']['title']['expose']['operator'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['identifier'] = 'title';
  $handler->display->display_options['filters']['title']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'admin/springboard/settings/config/designations';

  $views[$view->name] = $view;

  // Return all the views.
  return $views;
}