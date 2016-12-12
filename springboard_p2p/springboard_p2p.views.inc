<?php
/**
 * @file
 * Provides Springboard P2P data and relationships to Views.
 */

/**
 * Implements hook_views_data().
 */
function springboard_p2p_views_data() {
  $data = array();

  $data['springboard_p2p_campaign_progress'] = array(
    'table' => array(
      'group' => t('P2P'),
      'join' => array(
        'node' => array(
          'left_field' => 'nid',
          'field' => 'nid',
        ),
      ),
    ),
    'progress' => array(
      'title' => t('Campaign progress'),
      'help' => t('The campaign progress'),
      'field' => array(
        'handler' => 'springboard_p2p_handler_field_campaign_progress',
        'click sortable' => FALSE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    ),
  );

  $data['springboard_p2p_campaign_percent_complete'] = array(
    'table' => array(
      'group' => t('P2P'),
      'join' => array(
        'node' => array(
          'left_field' => 'nid',
          'field' => 'nid',
        ),
      ),
    ),
    'percent_complete' => array(
      'title' => t('Campaign percent complete'),
      'help' => t('The campaign percent complete'),
      'field' => array(
        'handler' => 'springboard_p2p_handler_field_campaign_progress_percent_complete',
        'click sortable' => FALSE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    ),
  );

  $data['springboard_p2p_private_campaign_approval'] = array(
    'table' => array(
      'group' => t('P2P'),
      'base' => array(
        'field' => 'nid',
        'title' => t('P2P private campaign approvals'),
        'help' => t('Users waiting for approval or approvaed to create content in a private P2P campaign.'),
        'weight' => -10,
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
      ),

    ),
    'nid' => array(
      'title' => t('Private P2P campaign'),
      'help' => t('The private P2P campaign.'),
      'relationship' => array(
        'base' => 'node',
        'base field' => 'nid',
        'handler' => 'views_handler_relationship',
        'label' => t('Private P2P campaign'),
        'title' => t('Private P2P campaign'),
        'help' => t('The private P2P campaign.'),
      ),
    ),
    'uid' => array(
      'title' => t('User requesting approval'),
      'help' => t('User requesting approval.'),
      'relationship' => array(
        'base' => 'users',
        'base field' => 'uid',
        'handler' => 'views_handler_relationship',
        'label' => t('User requesting approval'),
        'title' => t('User requesting approval.'),
        'help' => t('User requesting approval to the private campaign.'),
      ),
    ),
    'status' => array(
      'title' => t('Approval status'),
      'help' => t('The state of the approval request.'),
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
    ),
    'created' => array(
      'title' => t('Requested time'),
      'help' => t('The time that the user requested approval.'),
      'field' => array(
        'handler' => 'views_handler_field_date',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort_date',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_date',
      ),
    ),
    'operations' => array(
      'field' => array(
        'title' => t('Operations links'),
        'help' => t('Display approve and reject links.'),
        'handler' => 'springboard_p2p_handler_field_campaign_approval_operations',
      ),
    ),
  );

  $data['springboard_p2p_personal_campaign_action'] = array(
    'table' => array(
      'group' => t('P2P action'),
      'base' => array(
        'field' => 'personal_campaign_nid',
        'title' => t('P2P personal campaign node'),
        'help' => t('Personal campaign node.'),
        'weight' => -10,
      ),
      'join' => array(
        'node' => array(
          'left_field' => 'nid',
          'field' => 'personal_campaign_nid',
        ),
        'users' => array(
          'left_field' => 'uid',
          'field' => 'personal_campaign_uid',
        ),
      ),
    ),
    'category_nid' => array(
      'title' => t('P2P Category'),
      'help' => t('The P2P category.'),
      'relationship' => array(
        'base' => 'node',
        'base field' => 'nid',
        'handler' => 'views_handler_relationship',
        'label' => t('P2P Category'),
        'title' => t('P2P Category'),
        'help' => t('The P2P Category.'),
      ),
    ),
    'campaign_nid' => array(
      'title' => t('P2P campaign'),
      'help' => t('The P2P campaign.'),
      'relationship' => array(
        'base' => 'node',
        'base field' => 'nid',
        'handler' => 'views_handler_relationship',
        'label' => t('P2P campaign'),
        'title' => t('P2P campaign'),
        'help' => t('The P2P campaign.'),
      ),
    ),
    'personal_campaign_nid' => array(
      'title' => t('P2P personal campaign'),
      'help' => t('The P2P personal campaign.'),
      'relationship' => array(
        'base' => 'node',
        'base field' => 'nid',
        'handler' => 'views_handler_relationship',
        'label' => t('P2P personal campaign'),
        'title' => t('P2P personal campaign'),
        'help' => t('The P2P personal campaign.'),
      ),
    ),
    'personal_campaign_uid' => array(
      'title' => t('Personal campaign user'),
      'help' => t('The user who owns this personal campaign.'),
      'relationship' => array(
        'base' => 'users',
        'base field' => 'uid',
        'handler' => 'views_handler_relationship',
        'label' => t('User requesting approval'),
        'title' => t('User requesting approval.'),
        'help' => t('User who owns this personal campaign.'),
      ),
    ),
    'form_nid' => array(
      'title' => t('P2P campaign form'),
      'help' => t('The P2P campaign form.'),
      'relationship' => array(
        'base' => 'node',
        'base field' => 'nid',
        'handler' => 'views_handler_relationship',
        'label' => t('Campaign form'),
        'title' => t('Campaign form'),
        'help' => t('The P2P personal campaign.'),
      ),
    ),
    'action_type' => array(
      'title' => t('Action type'),
      'help' => t('What type of action was taken by the user.'),
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
    ),
    'uid' => array(
      'title' => t('User'),
      'help' => t('User who acted on the form.'),
      'relationship' => array(
        'base' => 'users',
        'base field' => 'uid',
        'handler' => 'views_handler_relationship',
        'label' => t('User'),
        'title' => t('User.'),
        'help' => t('User who acted on the form.'),
      ),
    ),
    'amount' => array(
      'title' => t('Action amount'),
      'help' => t('Action amount.'),
      'field' => array(
        'handler' => 'springboard_p2p_handler_p2p_action_amount',
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
    ),
    'created' => array(
      'title' => t('Date'),
      'help' => t('The date.'),
      'field' => array(
        'handler' => 'views_handler_field_date',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort_date',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_date',
      ),
    ),
    'status' => array(
      'title' => t('Status'),
      'help' => t('Whether the personal campaign action is visible.'),
      'field' => array(
        'handler' => 'views_handler_field_boolean',
        'click sortable' => TRUE,
        'output formats' => array(
          'visible-notvisible' => array(t('Visible'), t('Not Visible')),
        ),
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_boolean_operator',
        'label' => t('Visible'),
        'type' => 'yes-no',
        'use equal' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
    ),
    'donor_name' => array(
      'title' => t('Donor Name'),
      'help' => t('Name of user who donated to the campaign.'),
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
    ),
  );

  return $data;
}
