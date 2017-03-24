<?php
/**
 * @file
 * Provides default view configurations.
 */

/**
 * Implements hook_views_default_views().
 */
function springboard_target_views_default_views() {

  $views = array();

  $view = new view();
  $view->name = 'sba_custom_targets';
  $view->description = 'Advocacy Custom Target and Custom Groups lists and search.';
  $view->tag = 'Advocacy';
  $view->base_table = 'advocacy';
  $view->human_name = 'SBA Custom Targets';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Custom Targets';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'springboard_target_views_access';
  $handler->display->display_options['access']['perm'] = 'view targets';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['submit_button'] = 'Search';
  $handler->display->display_options['pager']['type'] = 'springboard_advocacy_views_plugin_pager';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'name' => 'name',
    'group_actions' => 'group_actions',
  );
  $handler->display->display_options['style_options']['default'] = 'name';
  $handler->display->display_options['style_options']['info'] = array(
    'name' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'group_actions' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['label'] = 'No results';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = '<h4>No custom targets have been created yet. </h4>
<p>You can <a href="/springboard/advocacy/targets/custom-targets/add">create one</a>.</p>';
  $handler->display->display_options['empty']['area']['format'] = 'full_html';
  /* Field: Advocacy: Salutation */
  $handler->display->display_options['fields']['salutation']['id'] = 'salutation';
  $handler->display->display_options['fields']['salutation']['table'] = 'advocacy';
  $handler->display->display_options['fields']['salutation']['field'] = 'salutation';
  /* Field: Advocacy: First Name */
  $handler->display->display_options['fields']['first_name']['id'] = 'first_name';
  $handler->display->display_options['fields']['first_name']['table'] = 'advocacy';
  $handler->display->display_options['fields']['first_name']['field'] = 'first_name';
  /* Field: Advocacy: Last Name */
  $handler->display->display_options['fields']['last_name']['id'] = 'last_name';
  $handler->display->display_options['fields']['last_name']['table'] = 'advocacy';
  $handler->display->display_options['fields']['last_name']['field'] = 'last_name';
  /* Field: Advocacy: Email */
  $handler->display->display_options['fields']['email']['id'] = 'email';
  $handler->display->display_options['fields']['email']['table'] = 'advocacy';
  $handler->display->display_options['fields']['email']['field'] = 'email';
  /* Field: Advocacy: Politcal Party */
  $handler->display->display_options['fields']['party']['id'] = 'party';
  $handler->display->display_options['fields']['party']['table'] = 'advocacy';
  $handler->display->display_options['fields']['party']['field'] = 'party';
  /* Field: Advocacy: Role */
  $handler->display->display_options['fields']['role']['id'] = 'role';
  $handler->display->display_options['fields']['role']['table'] = 'advocacy';
  $handler->display->display_options['fields']['role']['field'] = 'role';
  /* Field: Advocacy: State */
  $handler->display->display_options['fields']['state']['id'] = 'state';
  $handler->display->display_options['fields']['state']['table'] = 'advocacy';
  $handler->display->display_options['fields']['state']['field'] = 'state';
  /* Filter criterion: Advocacy: Search By Type */
  $handler->display->display_options['filters']['search_class_name']['id'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_class_name']['field'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['value'] = array(
    'Target' => 'Target',
  );
  $handler->display->display_options['filters']['search_class_name']['expose']['operator_id'] = 'search_class_name_op';
  $handler->display->display_options['filters']['search_class_name']['expose']['label'] = 'Search By Type';
  $handler->display->display_options['filters']['search_class_name']['expose']['operator'] = 'search_class_name_op';
  $handler->display->display_options['filters']['search_class_name']['expose']['identifier'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['expose']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_class_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  /* Filter criterion: Advocacy: Last Name */
  $handler->display->display_options['filters']['last_name']['id'] = 'last_name';
  $handler->display->display_options['filters']['last_name']['table'] = 'advocacy';
  $handler->display->display_options['filters']['last_name']['field'] = 'last_name';
  $handler->display->display_options['filters']['last_name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['last_name']['expose']['operator_id'] = 'last_name_op';
  $handler->display->display_options['filters']['last_name']['expose']['label'] = 'Last Name';
  $handler->display->display_options['filters']['last_name']['expose']['operator'] = 'last_name_op';
  $handler->display->display_options['filters']['last_name']['expose']['identifier'] = 'last_name';
  $handler->display->display_options['filters']['last_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  /* Filter criterion: Advocacy: Search By Gender */
  $handler->display->display_options['filters']['search_gender']['id'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_gender']['field'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['value'] = array(
    'M' => 'M',
    'F' => 'F',
  );
  $handler->display->display_options['filters']['search_gender']['exposed'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['expose']['operator_id'] = 'search_gender_op';
  $handler->display->display_options['filters']['search_gender']['expose']['label'] = 'Search By Gender';
  $handler->display->display_options['filters']['search_gender']['expose']['operator'] = 'search_gender_op';
  $handler->display->display_options['filters']['search_gender']['expose']['identifier'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['expose']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['search_gender']['expose']['reduce'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['is_grouped'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['group_info']['label'] = 'Search By Gender';
  $handler->display->display_options['filters']['search_gender']['group_info']['identifier'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['group_info']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['group_info']['group_items'] = array(
    1 => array(
      'title' => 'Male',
      'operator' => 'in',
      'value' => array(
        'F' => 'F',
        'all' => 0,
        'M' => 0,
      ),
    ),
    2 => array(
      'title' => 'Female',
      'operator' => 'in',
      'value' => array(
        'F' => 'F',
        'all' => 0,
        'M' => 0,
      ),
    ),
  );
  /* Filter criterion: Advocacy: Search By State */
  $handler->display->display_options['filters']['search_state']['id'] = 'search_state';
  $handler->display->display_options['filters']['search_state']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_state']['field'] = 'search_state';
  $handler->display->display_options['filters']['search_state']['value'] = array(
    'AL' => 'AL',
    'AK' => 'AK',
    'AZ' => 'AZ',
    'AR' => 'AR',
    'CA' => 'CA',
    'CO' => 'CO',
    'CT' => 'CT',
    'DE' => 'DE',
    'DC' => 'DC',
    'FL' => 'FL',
    'GA' => 'GA',
    'HI' => 'HI',
    'ID' => 'ID',
    'IL' => 'IL',
    'IN' => 'IN',
    'IA' => 'IA',
    'KS' => 'KS',
    'KY' => 'KY',
    'LA' => 'LA',
    'ME' => 'ME',
    'MD' => 'MD',
    'MA' => 'MA',
    'MI' => 'MI',
    'MN' => 'MN',
    'MS' => 'MS',
    'MO' => 'MO',
    'MT' => 'MT',
    'NE' => 'NE',
    'NV' => 'NV',
    'NH' => 'NH',
    'NJ' => 'NJ',
    'NM' => 'NM',
    'NY' => 'NY',
    'NC' => 'NC',
    'ND' => 'ND',
    'OH' => 'OH',
    'OK' => 'OK',
    'OR' => 'OR',
    'PA' => 'PA',
    'RI' => 'RI',
    'SC' => 'SC',
    'SD' => 'SD',
    'TN' => 'TN',
    'TX' => 'TX',
    'UT' => 'UT',
    'VT' => 'VT',
    'VA' => 'VA',
    'WA' => 'WA',
    'WV' => 'WV',
    'WI' => 'WI',
    'WY' => 'WY',
  );
  $handler->display->display_options['filters']['search_state']['exposed'] = TRUE;
  $handler->display->display_options['filters']['search_state']['expose']['operator_id'] = 'search_state_op';
  $handler->display->display_options['filters']['search_state']['expose']['label'] = 'Search By State';
  $handler->display->display_options['filters']['search_state']['expose']['operator'] = 'search_state_op';
  $handler->display->display_options['filters']['search_state']['expose']['identifier'] = 'search_state';
  $handler->display->display_options['filters']['search_state']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['search_state']['expose']['reduce'] = TRUE;
  /* Filter criterion: Advocacy: Search By Branch */
  $handler->display->display_options['filters']['search_role']['id'] = 'search_role';
  $handler->display->display_options['filters']['search_role']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_role']['field'] = 'search_role';
  $handler->display->display_options['filters']['search_role']['value'] = array(
    'SR|SS' => 'SR|SS',
    'FR|FS' => 'FR|FS',
  );
  $handler->display->display_options['filters']['search_role']['exposed'] = TRUE;
  $handler->display->display_options['filters']['search_role']['expose']['operator_id'] = 'search_role_op';
  $handler->display->display_options['filters']['search_role']['expose']['label'] = 'Search By Level';
  $handler->display->display_options['filters']['search_role']['expose']['operator'] = 'search_role_op';
  $handler->display->display_options['filters']['search_role']['expose']['identifier'] = 'search_role';
  $handler->display->display_options['filters']['search_role']['expose']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_role']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['search_role']['expose']['reduce'] = TRUE;
  $handler->display->display_options['filters']['search_role']['is_grouped'] = TRUE;
  $handler->display->display_options['filters']['search_role']['group_info']['label'] = 'Search By Level';
  $handler->display->display_options['filters']['search_role']['group_info']['identifier'] = 'search_role';
  $handler->display->display_options['filters']['search_role']['group_info']['widget'] = 'radios';
  $handler->display->display_options['filters']['search_role']['group_info']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_role']['group_info']['group_items'] = array(
    1 => array(
      'title' => 'State',
      'operator' => 'in',
      'value' => array(
        'SR|SS' => 'SR|SS',
        'all' => 0,
        'FR|FS' => 0,
      ),
    ),
    2 => array(
      'title' => 'Federal',
      'operator' => 'in',
      'value' => array(
        'FR|FS' => 'FR|FS',
        'all' => 0,
        'SR|SS' => 0,
      ),
    ),
  );

  /* Display: Custom Targets block */
  $handler = $view->new_display('block', 'Custom Targets block', 'block_1');
  $handler->display->display_options['defaults']['use_ajax'] = FALSE;
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['defaults']['exposed_form'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['reset_button'] = TRUE;
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'salutation' => 'salutation',
    'first_name' => 'first_name',
    'last_name' => 'last_name',
    'organization' => 'organization',
    'title' => 'title',
    'target_actions' => 'target_actions',
    'updated_at' => 'updated_at',
  );
  $handler->display->display_options['style_options']['default'] = 'updated_at';
  $handler->display->display_options['style_options']['info'] = array(
    'salutation' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'first_name' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'last_name' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'organization' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'title' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'target_actions' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'updated_at' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Advocacy: Salutation */
  $handler->display->display_options['fields']['salutation']['id'] = 'salutation';
  $handler->display->display_options['fields']['salutation']['table'] = 'advocacy';
  $handler->display->display_options['fields']['salutation']['field'] = 'salutation';
  /* Field: Advocacy: First Name */
  $handler->display->display_options['fields']['first_name']['id'] = 'first_name';
  $handler->display->display_options['fields']['first_name']['table'] = 'advocacy';
  $handler->display->display_options['fields']['first_name']['field'] = 'first_name';
  /* Field: Advocacy: Last Name */
  $handler->display->display_options['fields']['last_name']['id'] = 'last_name';
  $handler->display->display_options['fields']['last_name']['table'] = 'advocacy';
  $handler->display->display_options['fields']['last_name']['field'] = 'last_name';
  /* Field: Advocacy: Organization */
  $handler->display->display_options['fields']['organization']['id'] = 'organization';
  $handler->display->display_options['fields']['organization']['table'] = 'advocacy';
  $handler->display->display_options['fields']['organization']['field'] = 'organization';
  $handler->display->display_options['fields']['organization']['label'] = 'Organization/Affiliation';
  /* Field: Advocacy: Organization title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'advocacy';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['label'] = 'Title/Role';
  /* Field: Advocacy: Updated Date */
  $handler->display->display_options['fields']['updated_at']['id'] = 'updated_at';
  $handler->display->display_options['fields']['updated_at']['table'] = 'advocacy';
  $handler->display->display_options['fields']['updated_at']['field'] = 'updated_at';
  $handler->display->display_options['fields']['updated_at']['label'] = 'Updated';
  /* Field: Advocacy: Target Actions */
  $handler->display->display_options['fields']['target_actions']['id'] = 'target_actions';
  $handler->display->display_options['fields']['target_actions']['table'] = 'advocacy';
  $handler->display->display_options['fields']['target_actions']['field'] = 'target_actions';
  $handler->display->display_options['fields']['target_actions']['label'] = 'Actions';
  $handler->display->display_options['fields']['target_actions']['which_links'] = array(
    'edit' => 'edit',
    'delete' => 'delete',
    'view' => 0,
  );
  $handler->display->display_options['defaults']['sorts'] = FALSE;
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Advocacy: Search Multiple Target Fields by Keywords */
  $handler->display->display_options['filters']['combine']['id'] = 'combine';
  $handler->display->display_options['filters']['combine']['table'] = 'advocacy';
  $handler->display->display_options['filters']['combine']['field'] = 'combine';
  $handler->display->display_options['filters']['combine']['group'] = 1;
  $handler->display->display_options['filters']['combine']['exposed'] = TRUE;
  $handler->display->display_options['filters']['combine']['expose']['operator_id'] = 'combine_op';
  $handler->display->display_options['filters']['combine']['expose']['label'] = 'Name or Organization Name';
  $handler->display->display_options['filters']['combine']['expose']['operator'] = 'combine_op';
  $handler->display->display_options['filters']['combine']['expose']['identifier'] = 'combine';
  $handler->display->display_options['filters']['combine']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['combine']['fields'] = array(
    'first_name' => 'first_name',
    'last_name' => 'last_name',
    'organization' => 'organization',
  );
  /* Filter criterion: Advocacy: Search By Type */
  $handler->display->display_options['filters']['search_class_name']['id'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_class_name']['field'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['value'] = array(
    'Target' => 'Target',
  );
  $handler->display->display_options['filters']['search_class_name']['group'] = 1;
  $handler->display->display_options['filters']['search_class_name']['expose']['operator_id'] = 'search_class_name_op';
  $handler->display->display_options['filters']['search_class_name']['expose']['label'] = 'Search By Type';
  $handler->display->display_options['filters']['search_class_name']['expose']['operator'] = 'search_class_name_op';
  $handler->display->display_options['filters']['search_class_name']['expose']['identifier'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['expose']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_class_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  /* Filter criterion: Advocacy: Search By Gender */
  $handler->display->display_options['filters']['search_gender']['id'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_gender']['field'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['value'] = array(
    'M' => 'M',
    'F' => 'F',
    'O' => 'O',
  );
  $handler->display->display_options['filters']['search_gender']['group'] = 1;
  $handler->display->display_options['filters']['search_gender']['exposed'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['expose']['operator_id'] = 'search_gender_op';
  $handler->display->display_options['filters']['search_gender']['expose']['label'] = 'Search By Gender';
  $handler->display->display_options['filters']['search_gender']['expose']['operator'] = 'search_gender_op';
  $handler->display->display_options['filters']['search_gender']['expose']['identifier'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['search_gender']['expose']['reduce'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['group_info']['label'] = 'Search By Gender';
  $handler->display->display_options['filters']['search_gender']['group_info']['identifier'] = 'search_gender';
  $handler->display->display_options['filters']['search_gender']['group_info']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_gender']['group_info']['group_items'] = array(
    1 => array(
      'title' => 'Male',
      'operator' => 'in',
      'value' => array(
        'M' => 'M',
        'all' => 0,
        'F' => 0,
        0 => 0,
      ),
    ),
    2 => array(
      'title' => 'Female',
      'operator' => 'in',
      'value' => array(
        'F' => 'F',
        'all' => 0,
        'M' => 0,
        0 => 0,
      ),
    ),
    3 => array(
      'title' => 'Other',
      'operator' => 'in',
      'value' => array(
        0 => '0',
        'all' => 0,
        'M' => 0,
        'F' => 0,
      ),
    ),
  );
  /* Filter criterion: Advocacy: Search By State */
  $handler->display->display_options['filters']['search_state']['id'] = 'search_state';
  $handler->display->display_options['filters']['search_state']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_state']['field'] = 'search_state';
  $handler->display->display_options['filters']['search_state']['value'] = array(
    'AL' => 'AL',
    'AK' => 'AK',
    'AZ' => 'AZ',
    'AR' => 'AR',
    'CA' => 'CA',
    'CO' => 'CO',
    'CT' => 'CT',
    'DE' => 'DE',
    'DC' => 'DC',
    'FL' => 'FL',
    'GA' => 'GA',
    'HI' => 'HI',
    'ID' => 'ID',
    'IL' => 'IL',
    'IN' => 'IN',
    'IA' => 'IA',
    'KS' => 'KS',
    'KY' => 'KY',
    'LA' => 'LA',
    'ME' => 'ME',
    'MD' => 'MD',
    'MA' => 'MA',
    'MI' => 'MI',
    'MN' => 'MN',
    'MS' => 'MS',
    'MO' => 'MO',
    'MT' => 'MT',
    'NE' => 'NE',
    'NV' => 'NV',
    'NH' => 'NH',
    'NJ' => 'NJ',
    'NM' => 'NM',
    'NY' => 'NY',
    'NC' => 'NC',
    'ND' => 'ND',
    'OH' => 'OH',
    'OK' => 'OK',
    'OR' => 'OR',
    'PA' => 'PA',
    'RI' => 'RI',
    'SC' => 'SC',
    'SD' => 'SD',
    'TN' => 'TN',
    'TX' => 'TX',
    'UT' => 'UT',
    'VT' => 'VT',
    'VA' => 'VA',
    'WA' => 'WA',
    'WV' => 'WV',
    'WI' => 'WI',
    'WY' => 'WY',
  );
  $handler->display->display_options['filters']['search_state']['group'] = 1;
  $handler->display->display_options['filters']['search_state']['exposed'] = TRUE;
  $handler->display->display_options['filters']['search_state']['expose']['operator_id'] = 'search_state_op';
  $handler->display->display_options['filters']['search_state']['expose']['label'] = 'Search By State';
  $handler->display->display_options['filters']['search_state']['expose']['operator'] = 'search_state_op';
  $handler->display->display_options['filters']['search_state']['expose']['identifier'] = 'search_state';
  $handler->display->display_options['filters']['search_state']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  $handler->display->display_options['filters']['search_state']['expose']['reduce'] = TRUE;

  /* Display: Custom Groups block */
  $handler = $view->new_display('block', 'Custom Groups block', 'block_2');
  $handler->display->display_options['defaults']['title'] = FALSE;
  $handler->display->display_options['title'] = 'Custom Groups';
  $handler->display->display_options['defaults']['use_ajax'] = FALSE;
  $handler->display->display_options['use_ajax'] = TRUE;
  $handler->display->display_options['defaults']['exposed_form'] = FALSE;
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['reset_button'] = TRUE;
  $handler->display->display_options['defaults']['pager'] = FALSE;
  $handler->display->display_options['pager']['type'] = 'springboard_advocacy_views_plugin_pager';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '1';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['defaults']['style_plugin'] = FALSE;
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'group_name' => 'group_name',
    'updated_at' => 'updated_at',
    'group_actions' => 'group_actions',
  );
  $handler->display->display_options['style_options']['default'] = 'updated_at';
  $handler->display->display_options['style_options']['info'] = array(
    'group_name' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'updated_at' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'group_actions' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  $handler->display->display_options['defaults']['style_options'] = FALSE;
  $handler->display->display_options['defaults']['row_plugin'] = FALSE;
  $handler->display->display_options['defaults']['row_options'] = FALSE;
  $handler->display->display_options['defaults']['empty'] = FALSE;
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['label'] = 'No results';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = '<h4>No custom groups have been created yet. </h4>
<p>You can <a href="/springboard/advocacy/targets/custom-groups/add">create one</a>.</p>';
  $handler->display->display_options['empty']['area']['format'] = 'full_html';
  $handler->display->display_options['defaults']['fields'] = FALSE;
  /* Field: Advocacy: Group Name */
  $handler->display->display_options['fields']['group_name']['id'] = 'group_name';
  $handler->display->display_options['fields']['group_name']['table'] = 'advocacy';
  $handler->display->display_options['fields']['group_name']['field'] = 'group_name';
  $handler->display->display_options['fields']['group_name']['element_label_colon'] = FALSE;
  /* Field: Advocacy: Child Groups */
  $handler->display->display_options['fields']['child_groups']['id'] = 'child_groups';
  $handler->display->display_options['fields']['child_groups']['table'] = 'advocacy';
  $handler->display->display_options['fields']['child_groups']['field'] = 'child_groups';
  /* Field: Advocacy: Matching targets */
  $handler->display->display_options['fields']['matching_targets']['id'] = 'matching_targets';
  $handler->display->display_options['fields']['matching_targets']['table'] = 'advocacy';
  $handler->display->display_options['fields']['matching_targets']['field'] = 'matching_targets';
  /* Field: Advocacy: Updated Date */
  $handler->display->display_options['fields']['updated_at']['id'] = 'updated_at';
  $handler->display->display_options['fields']['updated_at']['table'] = 'advocacy';
  $handler->display->display_options['fields']['updated_at']['field'] = 'updated_at';
  $handler->display->display_options['fields']['updated_at']['label'] = 'Updated';
  /* Field: Advocacy: Group Actions */
  $handler->display->display_options['fields']['group_actions']['id'] = 'group_actions';
  $handler->display->display_options['fields']['group_actions']['table'] = 'advocacy';
  $handler->display->display_options['fields']['group_actions']['field'] = 'group_actions';
  $handler->display->display_options['fields']['group_actions']['label'] = 'Actions';
  $handler->display->display_options['fields']['group_actions']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['group_actions']['which_links'] = array(
    0 => 'view',
    1 => 'edit',
    2 => 'delete',
    'edit' => 'edit',
    'delete' => 'delete',
    'view' => 0,
  );
  $handler->display->display_options['defaults']['filter_groups'] = FALSE;
  $handler->display->display_options['defaults']['filters'] = FALSE;
  /* Filter criterion: Advocacy: Search By Type */
  $handler->display->display_options['filters']['search_class_name']['id'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['table'] = 'advocacy';
  $handler->display->display_options['filters']['search_class_name']['field'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['value'] = array(
    'Group' => 'Group',
  );
  $handler->display->display_options['filters']['search_class_name']['group'] = 1;
  $handler->display->display_options['filters']['search_class_name']['expose']['operator_id'] = 'search_class_name_op';
  $handler->display->display_options['filters']['search_class_name']['expose']['label'] = 'Search By Type';
  $handler->display->display_options['filters']['search_class_name']['expose']['operator'] = 'search_class_name_op';
  $handler->display->display_options['filters']['search_class_name']['expose']['identifier'] = 'search_class_name';
  $handler->display->display_options['filters']['search_class_name']['expose']['multiple'] = TRUE;
  $handler->display->display_options['filters']['search_class_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
  );
  /* Filter criterion: Advocacy: Group Name */
  $handler->display->display_options['filters']['group_name']['id'] = 'group_name';
  $handler->display->display_options['filters']['group_name']['table'] = 'advocacy';
  $handler->display->display_options['filters']['group_name']['field'] = 'group_name';
  $handler->display->display_options['filters']['group_name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['group_name']['expose']['operator_id'] = 'group_name_op';
  $handler->display->display_options['filters']['group_name']['expose']['label'] = 'Group name or target name';
  $handler->display->display_options['filters']['group_name']['expose']['operator'] = 'group_name_op';
  $handler->display->display_options['filters']['group_name']['expose']['identifier'] = 'group_name';
  $handler->display->display_options['filters']['group_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
    4 => 0,
    5 => 0,
    6 => 0,
  );

  $views[$view->name] = $view;

  return $views;
}