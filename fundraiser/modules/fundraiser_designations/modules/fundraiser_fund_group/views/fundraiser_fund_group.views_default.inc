<?php

/**
 * @file
 * Provides default view configurations.
 */


/**
 * Implements hook_views_default_views().
 */
function fundraiser_fund_group_views_default_views() {
  $views = array();

  /******* Designations Landing page *************/

  $view = new view();
  $view->name = 'fundraiser_fund_group';
  $view->description = '';
  $view->tag = '';
  $view->base_table = 'fundraiser_fund_group';
  $view->human_name = 'Fund Groups';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Fund Groups';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '50';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'name' => 'name',
    'edit_fundraiser_fund_group' => 'edit_fundraiser_fund_group',
    'nid' => 'nid',
    'delete_fundraiser_fund_group' => 'edit_fundraiser_fund_group',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'name' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'edit_fundraiser_fund_group' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'nid' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '|',
      'empty_column' => 0,
    ),
    'delete_fundraiser_fund_group' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* Relationship: Entity Reference: Referenced Entity */
  $handler->display->display_options['relationships']['field_ffg_form_id_target_id']['id'] = 'field_ffg_form_id_target_id';
  $handler->display->display_options['relationships']['field_ffg_form_id_target_id']['table'] = 'field_data_field_ffg_form_id';
  $handler->display->display_options['relationships']['field_ffg_form_id_target_id']['field'] = 'field_ffg_form_id_target_id';
  /* Field: Fund group: Fund Group ID */
  $handler->display->display_options['fields']['field_ffg_form_id']['id'] = 'field_ffg_form_id';
  $handler->display->display_options['fields']['field_ffg_form_id']['table'] = 'field_data_field_ffg_form_id';
  $handler->display->display_options['fields']['field_ffg_form_id']['field'] = 'field_ffg_form_id';
  $handler->display->display_options['fields']['field_ffg_form_id']['settings'] = array(
    'link' => 0,
  );
  /* Contextual filter: Fund group: Fund Group ID (field_ffg_form_id) */
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['id'] = 'field_ffg_form_id_target_id';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['table'] = 'field_data_field_ffg_form_id';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['field'] = 'field_ffg_form_id_target_id';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['default_action'] = 'access denied';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['field_ffg_form_id_target_id']['summary_options']['items_per_page'] = '25';

  /* Display: Designations tab */
  $handler = $view->new_display('block', 'Designations tab', 'block_1');
  $handler->display->display_options['display_description'] = 'Embedded block for edit page';
  $handler->display->display_options['defaults']['use_ajax'] = FALSE;
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'name' => 'name',
    'draggableviews' => 'draggableviews',
    'delete_fundraiser_fund_group' => 'edit_fundraiser_fund_group',
    'edit_fundraiser_fund_group' => 'edit_fundraiser_fund_group',
    'override_fundraiser_fund_group' => 'name',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'name' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => ' ',
      'empty_column' => 0,
    ),
    'draggableviews' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'delete_fundraiser_fund_group' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'edit_fundraiser_fund_group' => array(
      'align' => '',
      'separator' => ' | ',
      'empty_column' => 0,
    ),
    'override_fundraiser_fund_group' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['empty'] = FALSE;
  /* No results behavior: Global: Unfiltered text */
  $handler->display->display_options['empty']['area_text_custom']['id'] = 'area_text_custom';
  $handler->display->display_options['empty']['area_text_custom']['table'] = 'views';
  $handler->display->display_options['empty']['area_text_custom']['field'] = 'area_text_custom';
  $handler->display->display_options['empty']['area_text_custom']['empty'] = TRUE;
  $handler->display->display_options['empty']['area_text_custom']['content'] = 'No fund groups have been added yet.';
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Fund group: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'fundraiser_fund_group';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  /* Field: Draggableviews: Fund group */
  $handler->display->display_options['fields']['draggableviews']['id'] = 'draggableviews';
  $handler->display->display_options['fields']['draggableviews']['table'] = 'fundraiser_fund_group';
  $handler->display->display_options['fields']['draggableviews']['field'] = 'draggableviews';
  $handler->display->display_options['fields']['draggableviews']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['draggableviews']['hide_alter_empty'] = FALSE;
  $handler->display->display_options['fields']['draggableviews']['draggableviews']['ajax'] = 0;
  /* Field: Fund group: Delete Link */
  $handler->display->display_options['fields']['delete_fundraiser_fund_group']['id'] = 'delete_fundraiser_fund_group';
  $handler->display->display_options['fields']['delete_fundraiser_fund_group']['table'] = 'fundraiser_fund_group';
  $handler->display->display_options['fields']['delete_fundraiser_fund_group']['field'] = 'delete_fundraiser_fund_group';
  /* Field: Fund group: Edit Link */
  $handler->display->display_options['fields']['edit_fundraiser_fund_group']['id'] = 'edit_fundraiser_fund_group';
  $handler->display->display_options['fields']['edit_fundraiser_fund_group']['table'] = 'fundraiser_fund_group';
  $handler->display->display_options['fields']['edit_fundraiser_fund_group']['field'] = 'edit_fundraiser_fund_group';
  $handler->display->display_options['fields']['edit_fundraiser_fund_group']['label'] = 'Actions';
  /* Field: Fund group: Override status */
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['id'] = 'override_fundraiser_fund_group';
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['table'] = 'fundraiser_fund_group';
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['field'] = 'override_fundraiser_fund_group';
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['label'] = 'Amounts Overridden?';
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['alter']['text'] = '([override_fundraiser_fund_group])';
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['override_fundraiser_fund_group']['text'] = 'blik';
  $handler->display->display_options['defaults']['sorts'] = FALSE;
  /* Sort criterion: Draggableviews: Weight */
  $handler->display->display_options['sorts']['weight']['id'] = 'weight';
  $handler->display->display_options['sorts']['weight']['table'] = 'draggableviews_structure';
  $handler->display->display_options['sorts']['weight']['field'] = 'weight';
  $handler->display->display_options['sorts']['weight']['order'] = 'DESC';
  $handler->display->display_options['sorts']['weight']['draggableviews_setting_view'] = 'self';
  $handler->display->display_options['sorts']['weight']['draggableviews_setting_new_items_bottom_list'] = 0;
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Fund group: Type */
  $handler->display->display_options['filters']['type']['id'] = 'type';
  $handler->display->display_options['filters']['type']['table'] = 'fundraiser_fund_group';
  $handler->display->display_options['filters']['type']['field'] = 'type';
  $handler->display->display_options['filters']['type']['value'] = array(
    'fundraiser_fund_group_default' => 'fundraiser_fund_group_default',
  );


  $views[$view->name] = $view;
  return $views;

}