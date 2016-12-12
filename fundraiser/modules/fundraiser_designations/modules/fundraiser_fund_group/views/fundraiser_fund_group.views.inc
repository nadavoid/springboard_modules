<?php

/**
 * @file
 * Providing extra functionality for the SBA Message UI via views.
 */


/**
 * Implements hook_views_data_alter().
 */
function fundraiser_fund_group_views_data_alter(&$data) {
  $data['fundraiser_fund_group']['link_fundraiser_fund_group'] = array(
    'field' => array(
      'title' => t('Link'),
      'help' => t('Provide a link to the fundraiser_fund_group.'),
      'handler' => 'fundraiser_fund_group_handler_link_field',
    ),
  );
  $data['fundraiser_fund_group']['edit_fundraiser_fund_group'] = array(
    'field' => array(
      'title' => t('Edit Link'),
      'help' => t('Provide a link to the edit form for the fundraiser_fund_group.'),
      'handler' => 'fundraiser_fund_group_handler_edit_link_field',
    ),
  );
  $data['fundraiser_fund_group']['delete_fundraiser_fund_group'] = array(
    'field' => array(
      'title' => t('Delete Link'),
      'help' => t('Provide a link to delete the fundraiser_fund_group.'),
      'handler' => 'fundraiser_fund_group_handler_delete_link_field',
    ),
  );
  $data['fundraiser_fund_group']['override_fundraiser_fund_group'] = array(
    'field' => array(
      'title' => t('Override status'),
      'help' => t('Shows a message regarding doantion amount override status.'),
      'handler' => 'fundraiser_fund_group_handler_override_field',
    ),
  );
}

