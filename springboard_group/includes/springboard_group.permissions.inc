<?php
/**
 * @file
 * Drupal and OG permission hooks.
 */

/**
 * Implements hook_permission().
 */
function springboard_group_permission() {
  return array(
    // Grants access to our custom groups admin ui.
    'configure springboard groups' => array(
      'title' => t('Configure Springboard Groups'),
      'description' => t('Access the Springboard Groups administration screens.'),
    ),

    // Access the Springboard Menu items.
    'access springboard groups dashboard' => array(
      'title' => t('Access Springboard Groups Dashboard'),
      'description' => t('View a list of all groups.'),
    ),

    // Grants access to the native OG admin UI, which we have re-permissioned
    // access to using hook_menu_alter, to hide it because it's confusing.
    // You need this permission to access it rather than
    // 'administer group'.
    'access og default admin ui' => array(
      'title' => t('Access Organic Groups Default Administration Screens'),
      'description' => t("Springboard Groups hides OG's default admin UI. This permission will unhide it."),
    ),

  );
}

/**
 * Implements hook_og_permission().
 */
function springboard_group_og_permission() {
  $items = array();

  $items['edit payment method'] = array(
    'title' => t('Administer group payment methods'),
    'description' => t("Edit a group's payments methods."),
    'roles' => array(OG_AUTHENTICATED_ROLE),
    'default role' => array(),
    'module' => 'commerce_payment',
  );

  $items['access all webform results'] = array(
    'title' => t('Access all webform results in group'),
    'description' => t('Grants access to the "Results" tab on all webforms in a group.'),
    'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
    'module' => 'webform',
  );

  $items['edit all webform submissions'] = array(
    'title' => t('Edit all webform submissions in group'),
    'description' => t('Allows editing of any webform submission in a group.'),
    'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
    'module' => 'webform',
  );

  $items['delete all webform submissions'] = array(
    'title' => t('Delete all webform submissions in group'),
    'description' => t('Allows deleting of any webform submission in a group.'),
    'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
    'module' => 'webform',
  );


  $group_types = springboard_group_get_grouped_types();

  foreach ($group_types as $type => $bundles) {
    if ($type == 'node') {
      $types = node_type_get_types();
      foreach ($bundles as $node_type => $field) {
        if (in_array($node_type, array_filter(variable_get('webform_node_types', [])))) {
          $items['access all ' . $node_type . ' webform results'] = array(
            'title' => t('Access all @type webform results in group', array('@type' => $types[$node_type]->name)),
            'description' => t('Grants access to the "Results" tab on all @type webforms in a group.', array('@type' => $types[$node_type]->name)),
            'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
            'module' => 'webform',
          );

          $items['edit all ' . $node_type . ' webform submissions'] = array(
            'title' => t('Edit all @type webform submissions in group', array('@type' => $types[$node_type]->name)),
            'description' => t('Allows editing of any @type webform submission in a group.', array('@type' => $types[$node_type]->name)),
            'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
            'module' => 'webform',
          );

          $items['delete all ' . $node_type . ' webform submissions'] = array(
            'title' => t('Delete all @type webform submissions in group', array('@type' => $types[$node_type]->name)),
            'description' => t('Allows deleting of any @type webform submission in a group.', array('@type' => $types[$node_type]->name)),
            'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
            'module' => 'webform',
          );
        }
      }
    }
  }
  return $items;
}

/**
 * Implements hook_og_permission_alter().
 *
 * Remove some default permissions for features we're not using.
 */
function springboard_group_og_permission_alter(&$perms) {
  $perms['subscribe']['default role'] = array();
  $perms['unsubscribe']['default role'] = array();
  $perms['administer group']['default role'] = array();
  $perms['approve and deny subscription']['default role'] = array();
}

/**
 * Implements hook_og_user_access_alter().
 *
 * Make groups that user is not subscribed to appear in the dropdown selector
 * if they have the correct permission.
 */
function springboard_group_og_user_access_alter(&$perm, $context) {
  if (user_access('administer group')) {
    // @todo don't need this probably.
    $perm['administer group'] = TRUE;
  }
}

/**
 * Implements hook_preprocess_TAG().
 *
 * Alter the groups permission page. Remove subscription permissions, rename
 * some items.
 */
function springboard_group_preprocess_og_ui_admin_permissions(&$vars) {

  if (!empty($vars['form']['group_type']) && $vars['form']['group_type']['#value'] == 'node') {
    if (!empty($vars['form']['gid']['#value'])) {
      $group = node_load($vars['form']['gid']['#value']);
    }
  }

  if ($vars['form']['bundle']['#value'] == 'springboard_group' || (!empty($group->type) && $group->type == 'springboard_group')) {

    // Array of permissions we remove from the UI because we don't need them.
    $omitted_perms = array(
      'manage roles',
      'manage permissions',
      'administer group',
      'subscribe',
      'subscribe without approval',
      'unsubscribe',
      'approve and deny subscription',
    );

    $form = $vars['form'];
    foreach (element_children($form['permission']) as $key) {
      if (is_numeric($key)) {
        // Rename the categories.
        switch ($vars['form']['permission'][$key]['#id']) {
          case 'og':
            $vars['form']['permission'][$key]['#markup'] = t('Groups Content');
            break;

          case 'og_ui':
            $vars['form']['permission'][$key]['#markup'] = t('Groups Members');
            break;

          case 'publishcontent':
            $vars['form']['permission'][$key]['#markup'] = t('Publication Status');
            break;

          case 'springboard_group':
            unset($vars['form']['permission'][$key]);
            break;

          case 'springboard_group_advocacy':
            $vars['form']['permission'][$key]['#markup'] = t('Advocacy');
            break;
        }
      }
      else {
        // Remove unneeded perms.
        if (in_array($key, $omitted_perms)) {
          unset($vars['form']['permission'][$key]);
        }
        foreach (element_children($form['checkboxes']) as $rid) {
          if (in_array($key, $omitted_perms)) {
            unset($vars['form']['checkboxes'][$rid][$key]);
          }
        }
        if ($key == 'update group') {
          $vars['form']['permission'][$key]['#description'] = t('Edit the group title, address, contact info, etc.');
        }
      }
    }
  }
}

/**
 * Add a help message to the top of the permission overview page.
 *
 * Called from springboard_group_page_alter().
 */
function springboard_group_permissions_overview__page_alter(&$page) {
  if (!empty($page['content']['system_main']['#form_id']) &&
    $page['content']['system_main']['#form_id'] == 'og_ui_admin_global_permissions'
  ) {

    $strict = variable_get('og_node_access_strict', TRUE);
    if ($strict) {
      $strict_message = t("Strict access control is enabled.");
    }
    else {
      $strict_message = t("Strict access control is not enabled.");
    }

    $help_message = t('More information about permissions is available on the help tab');
    $message = t("Administrator members inherit all permissions granted to members.");
    $help = array(
      'help_message' => array(
        '#markup' => $message . '<br />' . $strict_message . '<br />' . $help_message,
        '#weight' => -10,
        '#prefix' => '<br /><p>',
      ),
    );
    array_unshift($page['content'], $help);
  }

  if (!empty($page['content']['system_main']['#form_id']) &&
    $page['content']['system_main']['#form_id'] == 'user_admin_permissions'
  ) {
    springboard_group_check_role_conflicts();
    if (current_path() == 'admin/springboard/groups/settings/springboard-permissions') {
      $help_message = t('More information about permissions is available on the help tab');
      $help = array(
        'help_message' => array(
          '#markup' => t('This is a subset of the global permissions normally found on the !link page which are relevant to Springboard Groups.', array('!link' => l(t('user permissions'), 'admin/people/permissions'))) . '<br />' . $help_message,
          '#weight' => -10,
          '#prefix' => '<br /><p>',
        ),
      );
      array_unshift($page['content'], $help);
    }
  }
}

/**
 * Check for "bypass content access control" and other conflicts.
 *
 * @return array
 *   Roles with by permission.
 */
function springboard_group_check_role_conflicts() {
  $roles = user_roles();
  $role_permissions = user_role_permissions($roles);
  $bypass_node_access = [];
  $bypass_rules_access = [];
  $administer_groups = [];
  foreach ($role_permissions as $rid => $permissions) {
    foreach ($permissions as $key => $permission) {
      if ($key == 'bypass node access' && $permission == 1) {
        $bypass_node_access[$rid] = $roles[$rid];
      }
      if ($key == 'administer payment methods' && $permission == 1) {
        $admin_payment[$rid] = $roles[$rid];
      }
      if ($key == 'bypass rules access' && $permission == 1) {
        $bypass_rules_access[$rid] = $roles[$rid];
      }
      if ($key == 'administer group' && $permission == 1) {
        $administer_groups[$rid] = $roles[$rid];
      }
    }
  }

  $node_problem = array_diff($bypass_node_access, $administer_groups);
  if (!empty($node_problem)) {
    springboard_group_permissions_problem_message($node_problem, 'bypass node access');
  }

  $rules_bypass_problem = array_diff($bypass_rules_access, $administer_groups);
  if (!empty($rules_bypass_problem)) {
    springboard_group_permissions_problem_message($rules_bypass_problem, 'bypass rules access');

  }
}

/**
 * @param $problem
 * @param $type
 */
function springboard_group_permissions_problem_message($problem, $type) {
  $count = count($problem);
  $word = format_plural($count, 'role', 'roles');
  $bad_roles = implode(', ', $problem);
  drupal_set_message(t('Warning! Some Springboard roles have the "@type" permission, but not the "administer organic groups" permissions. This can cause problems. Please fix the following @word: @bad_roles.', array(
    '@word' => $word,
    '@bad_roles' => $bad_roles,
    '@type' => $type,
  )), 'error');
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Add the js which hides non-group permissions to our alternate version
 * of Drupal's permissions overview form. Alters some descriptions too.
 */
function springboard_group_form_user_admin_permissions_alter(&$form, $form_state) {
  if (isset($form['permission']['administer group'])) {
    $form['permission']['administer group']['#markup'] = ' Administer Organic groups.';
    $form['permission']['administer group']['#description'] = 'Administer and post in all groups.';
  }

  foreach ($form['permission'] as $key => $perm) {
    if (strpos($key, 'springboard_group') !== FALSE) {
      $form['permission'][$key]['#markup'] = str_replace('content', 'Springboard Group', $form['permission'][$key]['#markup']);
    }
  }
  if (arg(1) == 'springboard' && arg(2) == 'groups') {
    $path = drupal_get_path('module', 'springboard_group') . '/js/springboard-groups-permissions.js';
    $form['#attached']['js'][] = $path;
  }
}

/**
 * Implements hook_form_alter().
 */
function springboard_group_form_og_ui_admin_global_permissions_alter(&$form, &$form_state, $form_id) {
  if (arg(2) == 'groups') {
    $path = drupal_get_path('module', 'springboard_group') . '/js/springboard-groups-groups-permissions.js';
    $form['#attached']['js'][] = $path;

    $settings = array('sbaShowNonMember' => variable_get('springboard_group_show_non_member', 0));
    $form['#attached']['js'][] = array(
      'data' => $settings,
      'type' => 'setting',
    );
  }
}

/**
 * Implements hook_form_alter().
 */
function springboard_group_form_og_ui_admin_permissions_alter(&$form, &$form_state, $form_id) {
  if (arg(2) == 'group') {
    $path = drupal_get_path('module', 'springboard_group') . '/js/springboard-groups-groups-permissions.js';
    $form['#attached']['js'][] = $path;

    $settings = array('sbaShowNonMember' => variable_get('springboard_group_show_non_member', 0));
    $form['#attached']['js'][] = array(
      'data' => $settings,
      'type' => 'setting',
    );
  }
}


/**
 * Get the current users in group permissions.
 */
function springboard_group_get_group_member_perms($gid, $account = NULL) {
  $in_group_perms = [];
  if (empty($account)) {
    global $user;
    $account = $user;
  }
  if ($gid != '_all' && $gid != '_none') {
    $roles = og_get_user_roles('node', $gid, $account->uid, TRUE, TRUE);
    $perms = og_role_permissions($roles);
    // Users may have multiple roles in a group.
    foreach ($perms as $role_perm) {
      $role_perm = array_filter($role_perm);
      foreach ($role_perm as $key => $status) {
        $in_group_perms[$key] = $status;
      }
    }
  }
  return $in_group_perms;
}