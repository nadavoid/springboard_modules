<?php
/**
 * @file
 * Drupal and Springboard Menu hooks for Springboard Groups.
 */

/**
 * Implements hook_menu().
 */
function springboard_group_menu() {
  $items = array();

  // User listing pages.
  $items['node/%/group/people'] = array(
    'title' => 'People',
    'type' => MENU_DEFAULT_LOCAL_TASK,
  );

  // User listing pages.
  $items['node/%/group/add-people'] = array(
    'page callback' => 'drupal_get_form',
    'title' => 'Add members',
    'page arguments' => array('og_ui_add_users', 0, 1),
    'type' => MENU_LOCAL_TASK,
    'file path' => drupal_get_path('module', 'og_ui'),
    'file' => 'og_ui.admin.inc',
    'weight' => 5,
    'access callback' => 'og_ui_user_access_group',
    'access arguments' => array('add user', 0, 1),
  );


  // User listing pages.
  $items['node/%/group/bulk-add-people'] = array(
    'page callback' => 'springboard_group_bulk_add_page',
    'page arguments' => array(0, 1),
    'weight' => 100,
    'title' => 'Bulk Add Members',
    'access callback' => 'og_ui_user_access_group',
    'access arguments' => array('add user', 0,1),
    'file' => 'includes/springboard_group.pages.inc',
    'type' => MENU_LOCAL_TASK,
  );

  // If per group permission is enabled add permissions tab.
  if (field_info_instance('node', OG_DEFAULT_ACCESS_FIELD, 'springboard_group')) {
    $items['node/%/group/permissions'] = array(
      'title' => 'Permissions',
      'description' => 'Determine access to features by selecting permissions for roles.',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('og_ui_admin_permissions', 0, 1),
      'access callback' => 'og_ui_user_access_group',
      'access arguments' => array('manage permissions', 0, 1),
      'file' => 'og_ui.admin.inc',
      'file path' => drupal_get_path('module', 'og_ui'),
      'weight' => 200,
      'type' => MENU_LOCAL_TASK,
    );
  }

  $items['admin/springboard/groups/settings'] = array(
    'title' => 'Springboard Groups',
    'description' => 'Configuration settings for Springboard Group content',
    'page callback' => 'springboard_group_settings_page',
    'access arguments' => array('configure springboard groups'),
    'file' => 'includes/springboard_group.admin.inc',
    'type' => MENU_NORMAL_ITEM,
  );

  $items['admin/springboard/groups/settings/groups'] = array(
    'title' => 'Groups Content',
    'description' => 'Configuration settings for Springboard Group features',
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  );

  $items["admin/springboard/groups/settings/permissions"] = array(
    'page callback' => 'drupal_get_form',
    'page arguments' => array('og_ui_admin_global_permissions', 'node', '0', 'springboard_group'),
    'title' => 'Groups Permissions',
    'access arguments' => array('configure springboard groups'),
    'description' => 'Configure Springboard Groups permissions.',
    'weight' => -9,
    'file' => 'og_ui.admin.inc',
    'file path' => drupal_get_path('module', 'og_ui'),
    'type' => MENU_LOCAL_TASK,
  );

  // Custom path for user permissions.
  $items['admin/springboard/groups/settings/springboard-permissions'] = array(
    'title' => 'Springboard Permissions',
    'description' => 'Permissions for springboard taxonomy.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('user_admin_permissions'),
    'access arguments' => array('administer permissions'),
    'file path' => drupal_get_path('module', 'user'),
    'file' => 'user.admin.inc',
    'type' => MENU_LOCAL_TASK,
    'weight' => -8,
  );

  $items["admin/springboard/groups/settings/roles"] = array(
    'page callback' => 'drupal_get_form',
    'page arguments' => array('og_ui_admin_new_role', 'node', '0', 'springboard_group'),
    'title' => 'Group Roles',
    'access arguments' => array('configure springboard groups'),
    'description' => 'Configure Springboard Group roles.',
    'file' => 'og_ui.admin.inc',
    'file path' => drupal_get_path('module', 'og_ui'),
    'type' => MENU_LOCAL_TASK,
    'weight' => -7,
  );

  $items['admin/springboard/groups/settings/role/%/edit'] = array(
    'page callback' => 'drupal_get_form',
    'page arguments' => array('og_ui_admin_roles', '', '0', '', 5),
    'title callback' => 'og_ui_menu_bundle_roles_title_callback',
    'title arguments' => array('OG @type - @bundle - @role', '', '', 5),
    'access arguments' => array('configure springboard groups'),
    'description' => 'Configure Organic groups group type roles.',
    'file path' => drupal_get_path('module', 'og_ui'),
    'file' => 'og_ui.admin.inc',
  );

  $items["admin/springboard/groups/settings/advanced"] = array(
    'page callback' => 'drupal_get_form',
    'page arguments' => array('springboard_group_advanced_settings'),
    'title' => 'Settings',
    'access arguments' => array('configure springboard groups'),
    'description' => 'Configure Springboard Groups advanced settings.',
    'weight' => -7,
    'file' => 'includes/springboard_group.admin.inc',
    'type' => MENU_LOCAL_TASK,
  );

  $items["admin/springboard/groups/settings/help"] = array(
    'page callback' => 'springboard_group_help',
    'page arguments' => array('admin/springboard/groups/settings/help', NULL),
    'title' => 'Help',
    'access arguments' => array('configure springboard groups'),
    'description' => 'Configure Springboard Groups advanced settings.',
    'weight' => -6,
    'file' => 'includes/springboard_group.admin.inc',
    'type' => MENU_LOCAL_TASK,
  );


  // OG module doesn't define a default tab. We help it here.
  //@todo remve.
  $items['group/%/%/admin/people/all'] = array(
    'title' => 'People',
    'type' => MENU_DEFAULT_LOCAL_TASK,
  );

  return $items;
}

/**
 * Implements hook_menu_alter().
 *
 * Decouple the OG 'administer group' permission from the OG configuration
 * pages, so the perm only applies in a group node context. That is, hide the
 * OG administration pages from users with the permission 'Administer Group',
 * but still allow them full access to configuring all group nodes.
 *
 * To grant access to the native OG admin pages, use the permission
 * 'access og default admin ui' instead.
 */
function springboard_group_menu_alter(&$items) {
  foreach ($items as $path => $item) {
    if (strpos($path, 'admin/config/group') !== FALSE && isset($item['access arguments'])) {
      foreach ($item['access arguments'] as $key => $argument) {
        if ($argument == 'administer group' && (isset($item['title']) && $item['title'] != "Delete")) {
          $items[$path]['access arguments'][$key] = 'access og default admin ui';
        }
      }
    }
    // Swap out OG's group landing page for our own.
    if (strpos($path, 'node/%/group') !== FALSE && isset($item['page callback']) && $item['page callback'] == 'og_ui_group_admin_overview') {
      $items[$path]['page callback'] = 'springboard_group_og_ui_group_admin_overview';
      $items[$path]['title'] = t('Members');
    }
    if (strpos($path, 'group/add-people') !== FALSE) {
      $items[$path]['access callback'] = FALSE;
      unset($items[$path]['access arguments']);
    }
    // Custom access for payment method overview page.
    if ($path == 'admin/commerce/config/payment-methods') {
      $items[$path]['access callback'] = 'springboard_group_rules_payment_method_overview_access';
    }
  }

  $grouped_types = springboard_group_get_grouped_types();
  foreach ($grouped_types as $entity_type => $bundle) {
    $function = 'springboard_group__' . $entity_type . '__menu_alter';
    if (function_exists($function)) {
      $function($items);
    }
  }
}

function springboard_group_rules_payment_method_overview_access($orig_args) {
  $group_context = springboard_group_get_group_context();
  $role_permissions = array();
  if (!empty($group_context) && $group_context != '_none' && $group_context != '_all' && strpos($group_context, '_some|') !== FALSE) {
    $roles = og_get_user_roles('node', $group_context);
    $perms = og_role_permissions($roles);
    $role_permissions = array_keys(array_filter(reset($perms)));
  }
  if (user_access($orig_args) || in_array('edit payment method', $role_permissions)) {
    return TRUE;
  }
}

/**
 * Implements hook_springboard_admin_admin_menu_items_alter().
 *
 * Add the springboard group links to springboard admin.
 */
function springboard_group_springboard_admin_admin_menu_items_alter(&$items) {

  $items['admin/springboard/groups'] = array(
    'link_path' => 'admin/springboard/groups/my',
    'link_title' => t('Groups'),
    'menu_name' => 'springboard_admin_menu',
    'expanded' => 1,
    'customized' => 1,
    'weight' => 3,
    '_children' => array(),
  );

  $items['admin/springboard/groups']['_children']['admin/springboard/groups'] = array(
    'link_path' => 'admin/springboard/groups',
    'link_title' => 'All Groups',
    'menu_name' => 'springboard_admin_menu',
    'expanded' => 1,
    'customized' => 1,
    'weight' => -5,
  );

  $items['admin/springboard/groups']['_children']['admin/springboard/groups/my'] = array(
    'link_path' => 'admin/springboard/groups/my',
    'link_title' => 'My Groups',
    'menu_name' => 'springboard_admin_menu',
    'expanded' => 1,
    'customized' => 1,
    'weight' => -6,
  );

  $items['admin/springboard/groups']['_children']['admin/springboard/groups/settings'] = array(
    'link_path' => 'admin/springboard/groups/settings',
    'link_title' => 'Groups Settings',
    'menu_name' => 'springboard_admin_menu',
    'expanded' => 1,
    'customized' => 1,
    'weight' => 0,
  );
}


/**
 * Implements hook_springboard_admin_alias_patterns().
 *
 * Put groups paths under springboard backend theme.
 */
function springboard_group_springboard_admin_alias_patterns() {
  return array(
    'group/node/%/admin/people' => array(
      'path' => array(
        'regex' => '|^/group/node/([0-9]+)/admin/people|',
        'replacement' => 'group/node/$1/admin/people',
      ),
      'alias' => array(
        'regex' => '|^springboard/group/node/([0-9]+)/admin/people|',
        'replacement' => 'springboard/group/node/$1/admin/people',
      ),
    ),
    'group/node/%/admin/people/add-user' => array(
      'path' => array(
        'regex' => '|^/group/node/([0-9]+)/admin/people/add-user|',
        'replacement' => 'group/node/$1/admin/people/add-user',
      ),
      'alias' => array(
        'regex' => '|^springboard/group/node/([0-9]+)/admin/people/add-user|',
        'replacement' => 'springboard/group/node/$1/admin/people/add-user',
      ),
    ),
    'group/node/%/admin/people/bulk-add' => array(
      'path' => array(
        'regex' => '|^/group/node/([0-9]+)/admin/people/bulk-add|',
        'replacement' => 'group/node/$1/admin/people/bulk-add',
      ),
      'alias' => array(
        'regex' => '|^springboard/group/node/([0-9]+)/admin/people/bulk-add|',
        'replacement' => 'springboard/group/node/$1/admin/people/bulk-add',
      ),
    ),
    'group/node/%/admin/roles' => array(
      'path' => array(
        'regex' => '|^/group/node/([0-9]+)/admin/roles|',
        'replacement' => 'group/node/$1/admin/roles',
      ),
      'alias' => array(
        'regex' => '|^springboard/group/node/([0-9]+)/admin/roles|',
        'replacement' => 'springboard/group/node/$1/admin/roles',
      ),
    ),
    'group/node/%/admin/people/permissions' => array(
      'path' => array(
        'regex' => '|^/group/node/([0-9]+)/admin/permissions|',
        'replacement' => 'group/node/$1/admin/permissions',
      ),
      'alias' => array(
        'regex' => '|^springboard/group/node/([0-9]+)/admin/permissions|',
        'replacement' => 'springboard/group/node/$1/admin/permissions',
      ),
    ),
  );
}
