<?php
/**
 * @file
 * springboard_group.pages_default.inc
 *
 * Panels/Page manager integration.
 */

/**
 * Implements hook_default_page_manager_handlers().
 */
function springboard_group_default_page_manager_handlers() {
  $export = array();

  $handler = new stdClass();
  $handler->disabled = FALSE; /* Edit this to true to make a default handler disabled initially */
  $handler->api_version = 1;
  $handler->name = 'node_view_panel_context';
  $handler->task = 'node_view';
  $handler->subtask = '';
  $handler->handler = 'panel_context';
  $handler->weight = 0;
  $handler->conf = array(
    'title' => 'Springboard Group',
    'no_blocks' => 0,
    'pipeline' => 'standard',
    'css_id' => '',
    'css' => '',
    'contexts' => array(),
    'relationships' => array(),
    'access' => array(
      'plugins' => array(
        0 => array(
          'name' => 'og_is_node_group',
          'settings' => array(
            'is_group_content' => '1',
          ),
          'context' => 'argument_entity_id:node_1',
          'not' => FALSE,
        ),
      ),
      'logic' => 'and',
    ),
  );
  $display = new panels_display();
  $display->layout = 'twocol_stacked';
  $display->layout_settings = array();
  $display->panel_settings = array(
    'style_settings' => array(
      'default' => NULL,
      'top' => NULL,
      'left' => NULL,
      'right' => NULL,
      'bottom' => NULL,
    ),
  );
  $display->cache = array();
  $display->title = '';
  $display->uuid = 'fec6c733-f207-43e9-b455-b0c012c0413b';
  $display->content = array();
  $display->panels = array();
  $pane = new stdClass();
  $pane->pid = 'new-758a4fc7-fa64-4550-befc-dd43c908ee43';
  $pane->panel = 'left';
  $pane->type = 'views_panes';
  $pane->subtype = 'springboard_group_content_pane-panel_pane_1';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'context' => array(
      0 => 'argument_entity_id:node_1',
    ),
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 0;
  $pane->locks = array();
  $pane->uuid = '758a4fc7-fa64-4550-befc-dd43c908ee43';
  $display->content['new-758a4fc7-fa64-4550-befc-dd43c908ee43'] = $pane;
  $display->panels['left'][0] = 'new-758a4fc7-fa64-4550-befc-dd43c908ee43';
  $pane = new stdClass();
  $pane->pid = 'new-d8c2f5a7-fef5-4e2a-9f65-584373d5d58c';
  $pane->panel = 'right';
  $pane->type = 'entity_field';
  $pane->subtype = 'node:field_sb_group_address';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'label' => 'title',
    'formatter' => 'addressfield_default',
    'delta_limit' => 0,
    'delta_offset' => '0',
    'delta_reversed' => FALSE,
    'formatter_settings' => array(
      'use_widget_handlers' => 1,
      'format_handlers' => array(
        'address' => 'address',
      ),
    ),
    'context' => 'argument_entity_id:node_1',
    'override_title' => 1,
    'override_title_text' => '',
    'override_title_heading' => 'h3',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 0;
  $pane->locks = array();
  $pane->uuid = 'd8c2f5a7-fef5-4e2a-9f65-584373d5d58c';
  $display->content['new-d8c2f5a7-fef5-4e2a-9f65-584373d5d58c'] = $pane;
  $display->panels['right'][0] = 'new-d8c2f5a7-fef5-4e2a-9f65-584373d5d58c';
  $pane = new stdClass();
  $pane->pid = 'new-e385ca58-d7e4-4846-914b-fb6d40ac8389';
  $pane->panel = 'right';
  $pane->type = 'entity_field';
  $pane->subtype = 'node:field_sb_group_cname';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'label' => 'title',
    'formatter' => 'text_default',
    'delta_limit' => 0,
    'delta_offset' => '0',
    'delta_reversed' => FALSE,
    'formatter_settings' => array(),
    'context' => 'argument_entity_id:node_1',
    'override_title' => 1,
    'override_title_text' => 'Group Contact',
    'override_title_heading' => 'h3',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 1;
  $pane->locks = array();
  $pane->uuid = 'e385ca58-d7e4-4846-914b-fb6d40ac8389';
  $display->content['new-e385ca58-d7e4-4846-914b-fb6d40ac8389'] = $pane;
  $display->panels['right'][1] = 'new-e385ca58-d7e4-4846-914b-fb6d40ac8389';
  $pane = new stdClass();
  $pane->pid = 'new-7f3b0491-e9d3-4ff8-a914-ff7873464947';
  $pane->panel = 'right';
  $pane->type = 'entity_field';
  $pane->subtype = 'node:field_sb_group_cemail';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'label' => 'title',
    'formatter' => 'text_default',
    'delta_limit' => 0,
    'delta_offset' => '0',
    'delta_reversed' => FALSE,
    'formatter_settings' => array(),
    'context' => 'argument_entity_id:node_1',
    'override_title' => 1,
    'override_title_text' => '',
    'override_title_heading' => 'h2',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 2;
  $pane->locks = array();
  $pane->uuid = '7f3b0491-e9d3-4ff8-a914-ff7873464947';
  $display->content['new-7f3b0491-e9d3-4ff8-a914-ff7873464947'] = $pane;
  $display->panels['right'][2] = 'new-7f3b0491-e9d3-4ff8-a914-ff7873464947';
//  $pane = new stdClass();
//  $pane->pid = 'new-1b68fa93-a09a-4a29-aa9f-b30ae76095df';
//  $pane->panel = 'right';
//  $pane->type = 'node_create_links';
//  $pane->subtype = 'node_create_links';
//  $pane->shown = TRUE;
//  $pane->access = array();
//  $pane->configuration = array(
//    'context' => 'argument_entity_id:node_1',
//    'override_title' => 0,
//    'override_title_text' => '',
//    'field_name' => 'og_group_ref',
//    'types' => array(),
//  );
//  $pane->cache = array();
//  $pane->style = array(
//    'settings' => NULL,
//  );
//  $pane->css = array();
//  $pane->extras = array();
//  $pane->position = 3;
//  $pane->locks = '';
//  $pane->uuid = '1b68fa93-a09a-4a29-aa9f-b30ae76095df';
//  $display->content['new-1b68fa93-a09a-4a29-aa9f-b30ae76095df'] = $pane;
//  $display->panels['right'][3] = 'new-1b68fa93-a09a-4a29-aa9f-b30ae76095df';
  $pane = new stdClass();
  $pane->pid = 'new-8f842c02-b786-4d7f-967e-2e311877c28f';
  $pane->panel = 'right';
  $pane->type = 'views_panes';
  $pane->subtype = 'og_members-panel_pane_1';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'context' => array(
      0 => 'argument_entity_id:node_1',
    ),
    'override_title' => 0,
    'override_title_text' => 'Members',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 4;
  $pane->locks = '';
  $pane->uuid = '8f842c02-b786-4d7f-967e-2e311877c28f';
  $display->content['new-8f842c02-b786-4d7f-967e-2e311877c28f'] = $pane;
  $display->panels['right'][4] = 'new-8f842c02-b786-4d7f-967e-2e311877c28f';
  $pane = new stdClass();
  $pane->pid = 'new-3122bedb-c66f-4f87-8834-7c468230d046';
  $pane->panel = 'right';
  $pane->type = 'entity_field';
  $pane->subtype = 'node:group_group';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'label' => 'title',
    'formatter' => 'og_group_subscribe',
    'formatter_settings' => array(
      'field_name' => 'og_user_group_ref',
    ),
    'context' => 'argument_entity_id:node_1',
    'override_title' => 1,
    'override_title_text' => '',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 5;
  $pane->locks = '';
  $pane->uuid = '3122bedb-c66f-4f87-8834-7c468230d046';
  $display->content['new-3122bedb-c66f-4f87-8834-7c468230d046'] = $pane;
  $display->panels['right'][5] = 'new-3122bedb-c66f-4f87-8834-7c468230d046';
  $pane = new stdClass();
  $pane->pid = 'new-cd20d04c-b1e7-4903-99a7-d1c4644efe7d';
  $pane->panel = 'top';
  $pane->type = 'entity_field';
  $pane->subtype = 'node:field_sb_group_logo';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'label' => 'title',
    'formatter' => 'image',
    'delta_limit' => 0,
    'delta_offset' => '0',
    'delta_reversed' => FALSE,
    'formatter_settings' => array(
      'image_style' => '',
      'image_link' => '',
    ),
    'context' => 'argument_entity_id:node_1',
    'override_title' => 1,
    'override_title_text' => '',
    'override_title_heading' => 'h2',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 0;
  $pane->locks = array();
  $pane->uuid = 'cd20d04c-b1e7-4903-99a7-d1c4644efe7d';
  $display->content['new-cd20d04c-b1e7-4903-99a7-d1c4644efe7d'] = $pane;
  $display->panels['top'][0] = 'new-cd20d04c-b1e7-4903-99a7-d1c4644efe7d';
  $display->hide_title = PANELS_TITLE_FIXED;
  $display->title_pane = '0';
  $handler->conf['display'] = $display;
  $export['node_view_panel_context'] = $handler;

  return $export;
}
