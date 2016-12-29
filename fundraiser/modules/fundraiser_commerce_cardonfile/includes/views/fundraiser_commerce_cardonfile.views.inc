<?php

/**
 * @file
 * Views integration for Fundraiser Commerce Card on File.
 */

/**
 * Implements hook_views_data_alter().
 */
function fundraiser_commerce_cardonfile_views_data_alter(&$data) {
  $data['commerce_cardonfile']['used_by'] = array(
    'title' => t('Used by'),
    'help' => t('What this card is used for, i.e. quick donations or recurring donations.'),
    'field' => array(
      'title' => t('Used By'),
      'help' => t('Display which fundraiser modules use this card.'),
      'handler' => 'fundraiser_commerce_cardonfile_handler_field_used_by',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_default_views_alter().
 *
 * Override the commerce_cardonfile module's cards view with some of our own
 * custom options. Exported from the adjusted view.
 */
function fundraiser_commerce_cardonfile_views_default_views_alter(&$views) {
  /* Display: Master */
  $handler = &$views['commerce_card_on_file_user_cards']->display['default']->handler;

  $handler->display->display_options = array();
  $handler->display->display_options['title'] = 'Stored cards';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'view own card data';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'none';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['style_options']['default_row_class'] = FALSE;
  $handler->display->display_options['style_options']['row_class_special'] = FALSE;
  $handler->display->display_options['row_plugin'] = 'fields';
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = 'You have no cards available to view.';
  $handler->display->display_options['empty']['area']['format'] = 'filtered_html';
  /* Relationship: Commerce Card on File: User uid */
  $handler->display->display_options['relationships']['user']['id'] = 'user';
  $handler->display->display_options['relationships']['user']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['relationships']['user']['field'] = 'user';
  $handler->display->display_options['relationships']['user']['required'] = TRUE;
  /* Field: Commerce Card on File: Card type */
  $handler->display->display_options['fields']['card_type']['id'] = 'card_type';
  $handler->display->display_options['fields']['card_type']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['fields']['card_type']['field'] = 'card_type';
  $handler->display->display_options['fields']['card_type']['label'] = '';
  $handler->display->display_options['fields']['card_type']['exclude'] = TRUE;
  $handler->display->display_options['fields']['card_type']['element_label_colon'] = FALSE;
  /* Field: Commerce Card on File: Card number */
  $handler->display->display_options['fields']['card_number']['id'] = 'card_number';
  $handler->display->display_options['fields']['card_number']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['fields']['card_number']['field'] = 'card_number';
  $handler->display->display_options['fields']['card_number']['label'] = '';
  $handler->display->display_options['fields']['card_number']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['card_number']['alter']['text'] = '[card_type] - [card_number]';
  $handler->display->display_options['fields']['card_number']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['card_number']['element_wrapper_type'] = 'h2';
  $handler->display->display_options['fields']['card_number']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['card_number']['format'] = 1;
  /* Field: Commerce Card on File: Card expiration */
  $handler->display->display_options['fields']['card_exp']['id'] = 'card_exp';
  $handler->display->display_options['fields']['card_exp']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['fields']['card_exp']['field'] = 'card_exp';
  $handler->display->display_options['fields']['card_exp']['label'] = 'Expires';
  $handler->display->display_options['fields']['card_exp']['element_default_classes'] = FALSE;
  /* Field: Commerce Card on File: Card name */
  $handler->display->display_options['fields']['card_name']['id'] = 'card_name';
  $handler->display->display_options['fields']['card_name']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['fields']['card_name']['field'] = 'card_name';
  $handler->display->display_options['fields']['card_name']['label'] = '';
  $handler->display->display_options['fields']['card_name']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['card_name']['element_default_classes'] = FALSE;
  /* Field: Commerce Card on File: Billing Profile */
  $handler->display->display_options['fields']['commerce_cardonfile_profile']['id'] = 'commerce_cardonfile_profile';
  $handler->display->display_options['fields']['commerce_cardonfile_profile']['table'] = 'field_data_commerce_cardonfile_profile';
  $handler->display->display_options['fields']['commerce_cardonfile_profile']['field'] = 'commerce_cardonfile_profile';
  $handler->display->display_options['fields']['commerce_cardonfile_profile']['label'] = 'Billing Information';
  if (module_exists('fundraiser_commerce_cardonfile')) {
    /* Field: Commerce Card on File: Used By */
    $handler->display->display_options['fields']['used_by']['id'] = 'used_by';
    $handler->display->display_options['fields']['used_by']['table'] = 'commerce_cardonfile';
    $handler->display->display_options['fields']['used_by']['field'] = 'used_by';
    $handler->display->display_options['fields']['used_by']['element_type'] = 'div';
    $handler->display->display_options['fields']['used_by']['element_label_type'] = 'span';
    $handler->display->display_options['fields']['used_by']['hide_empty'] = TRUE;
  }
  /* Field: Commerce Card on File: Operations links */
  $handler->display->display_options['fields']['operations']['id'] = 'operations';
  $handler->display->display_options['fields']['operations']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['fields']['operations']['field'] = 'operations';
  $handler->display->display_options['fields']['operations']['label'] = '';
  $handler->display->display_options['fields']['operations']['element_label_colon'] = FALSE;
  $handler->display->display_options['fields']['operations']['add_destination'] = 0;
  /* Sort criterion: Commerce Card on File: Instance default */
  $handler->display->display_options['sorts']['instance_default']['id'] = 'instance_default';
  $handler->display->display_options['sorts']['instance_default']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['sorts']['instance_default']['field'] = 'instance_default';
  /* Contextual filter: Commerce Card on File: User uid */
  $handler->display->display_options['arguments']['user']['id'] = 'user';
  $handler->display->display_options['arguments']['user']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['arguments']['user']['field'] = 'user';
  $handler->display->display_options['arguments']['user']['default_action'] = 'empty';
  $handler->display->display_options['arguments']['user']['exception']['value'] = '';
  $handler->display->display_options['arguments']['user']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['user']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['user']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['user']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['user']['specify_validation'] = TRUE;
  $handler->display->display_options['arguments']['user']['validate']['type'] = 'commerce_cof_owner';
  /* Filter criterion: Commerce Card on File: Card Status */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'commerce_cardonfile';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = array(
    1 => '1',
  );

  /* Display: Page */
  $handler = &$views['commerce_card_on_file_user_cards']->display['page']->handler;
  $handler->display->display_options['path'] = 'user/%/cards';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Stored cards';
  $handler->display->display_options['menu']['description'] = 'Manage your stored card data.';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['name'] = 'management';
  $handler->display->display_options['menu']['context'] = 0;
}

/**
 * Implements hook_views_data().
 */
function fundraiser_quick_donate_views_data() {
  $data['fundraiser_quick_donate_cardonfile']['table'] = array(
    'group' => t('Springboard Quick Donations'),
    'base' => array(
      'title' => t('Springboard Quck Donation Opt In'),
      'help' => t('Tracks users opted into quick donations.'),
    ),
    'join' => array(
      'node' => array(
        'left_field' => 'nid',
        'field' => 'nid',
      ),
      'users' => array(
        'left_field' => 'uid',
        'field' => 'uid',
      ),
      'commerce_cardonfile' => array(
        'left_field' => 'card_id',
        'field' => 'card_id',
      ),
      'fundraiser_donation' => array(
        'left_field' => 'did',
        'field' => 'did',
      ),
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['nid'] = array(
    'title' => t('Fundraiser node'),
    'help' => t('Fundraiser form node id.'),
    'relationship' => array(
      'base' => 'node',
      'base field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Fundraiser node'),
    ),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['uid'] = array(
    'title' => t('Quick donation opt-in user'),
    'help' => t('User associated with quick donation opt-in record.'),
    'relationship' => array(
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('Opt in user'),
    ),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['status'] = array(
    'title' => t('Quick donation opt-in status'),
    'help' => t('Whether the user is actively enrolled in quick donations.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['card_id'] = array(
    'title' => t('Quick donation opt-in card ID'),
    'help' => t('The card ID associated with the quick donation.'),
    'relationship' => array(
      'base' => 'commerce_cardonfile',
      'base field' => 'card_id',
      'handler' => 'views_handler_relationship',
      'label' => t('Opt in card ID'),
    ),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['did'] = array(
    'title' => t('Quick donation opt-in donation ID'),
    'help' => t('The donation ID associated with the quick donation.'),
    'relationship' => array(
      'base' => 'fundraiser_donation',
      'base field' => 'did',
      'handler' => 'views_handler_relationship',
      'label' => t('Opt in donation ID'),
    ),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['optin_date'] = array(
    'title' => t('Quick donation opt-in time'),
    'help' => t('Timestamp of when user opted into quick donations.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
  );
  $data['fundraiser_quick_donate_cardonfile']['ms'] = array(
    'title' => t('Quick donation market source'),
    'help' => t('Market source value for the quick donation record.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}
