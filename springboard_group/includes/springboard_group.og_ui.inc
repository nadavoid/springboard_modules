<?php

/**
 * Implements hook_og_ui_get_group_admin().
 *
 * Remove the permissions and roles links for the group node's group tab.
 */
function springboard_group_og_ui_get_group_admin_alter(&$data, $context) {
  if ($context['entity_type'] == 'node') {
    $node = node_load($context['etid']);
    if ($node->type == 'springboard_group') {
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Change the default text on og_group_ref field deletion confirm page. That is
 * when making an entity un-groupable.
 */
function springboard_group_form_og_ui_field_delete_form_alter(&$form, &$form_state) {
  if (isset($_GET['destination']) && $_GET['destination'] == 'admin/springboard/groups/settings') {
    $form['actions']['cancel']['#markup'] = l(t('cancel'), 'admin/springboard/groups/settings');
    $form['description']['#markup'] = t('All group associations for the @type content type will be deleted. This cannot be undone.', array('@type' => arg(5)));
    $form['#submit'][] = 'springboard_group_form_og_ui_field_delete_submit';
  }
}

/**
 * Delete multiple choice variable when deleting its owner field.
 */
function springboard_group_form_og_ui_field_delete_submit($form, $form_state) {
  if ($_GET['destination'] == 'admin/springboard/groups/settings') {
    variable_del('og_' . $form_state['values']['bundle'] . '_multiple');
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Remove the "request" field from the add user form. Not needed.
 */
function springboard_group_form_og_ui_add_users_alter(&$form, $form_state) {
  $form['membership_fields']['og_membership_request']['#access'] = FALSE;;
}

/**
 * Replace OG UI's default landing page with our own.
 *
 * If the group bundle is "springboard_group" replace the OG UI-provided
 * landing page (a set of links to the user management function) with the user
 * management page itself. Additional management links are converted to
 * secondary menu tabs in springboard_group_menu().
 *
 * @param string $entity_type
 *   The bundle type.
 * @param int $etid
 *   The node id.
 *
 * @return array
 *   The page render array.
 */
function springboard_group_og_ui_group_admin_overview($entity_type, $etid) {
  $content = '';
  og_set_breadcrumb($entity_type, $etid);
  $items = og_ui_get_group_admin($entity_type, $etid);
  if ($entity_type == 'node') {
    $node = node_load($etid);
  }
  // Not a springboard group, proceed as normal.
  if (empty($node) || $node->type != 'springboard_group') {
    foreach ($items as &$item) {
      $item['href'] = "group/$entity_type/$etid/" . $item['href'];
      $item['localized_options'] = array();
    }
    $form['admin_block_content'] = array(
      '#theme' => 'admin_block_content',
      '#content' => $items,
    );
    $content = $form;
  }
  // Springboard group, replace the page with user management.
  elseif (og_ui_user_access_group('manage members', $entity_type, $etid)) {
    $content = og_ui_admin_account($entity_type, $etid);
  }
  return $content;
}

/**
 * Implements hook_form_alter().
 */
function springboard_group_form_og_ui_delete_membership_alter(&$form, &$form_state, $form_id) {
  $form['#submit'][] = 'springboard_group_delete_membership_submit';
}


/**
 * Change the redirect to match our custom nav structure.
 */
function springboard_group_delete_membership_submit($form, &$form_state) {
  if (arg(1) == 'node') {
    $membership = $form_state['og_membership'];
    $node = node_load($membership->gid);
    if ($node->type == 'springboard_group') {
      $form_state['redirect'] = 'node/' . $membership->gid . '/group';
    }
  }
}

/**
 * @param $vars
 */
function springboard_group_preprocess_og_ui_admin_new_role(&$vars) {
  if (empty(variable_get('springboard_group_show_non_member', NULL)));
  foreach($vars['form']['role_names']['#value'] as $index => $role) {
    if ($role == 'non-member') {
      unset($vars['form']['role_names']['#value'][$index]);
    }
  }
}

/**
 * Implements hook_theme_registry_alter().
 *
 * Modify organic group's roles apge theme.
 */
function springboard_group_theme_registry_alter(&$theme_registry) {
  $theme_registry['og_ui_admin_new_role']['function'] = 'theme_springboard_group_admin_new_role';
}


/**
 * Modify og's roles theme for springboard groups.
 */
function theme_springboard_group_admin_new_role($vars) {
  $form = $vars['form'];
  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => 2));

  $group_type = $form['group_type']['#value'];
  $gid = $form['gid']['#value'];
  $bundle = $form['bundle']['#value'];
  $role_names = $form['role_names']['#value'];

  if ($gid) {
    $group = entity_load_single($group_type, $gid);
    $default_access = $group && og_is_group_default_access($group_type, $group);
  }
  else {
    $default_access = FALSE;
  }
  foreach ($role_names as $rid => $name) {
    $text = !$default_access ? t('edit permissions') : t('view permissions');
    if ($bundle != 'springboard_group') {
      $path = $gid ? "group/$group_type/$gid/admin" : 'admin/config/group';
      $permissions = l($text, "$path/permission/$rid/edit");
    }
    else {
      $path = $gid ? "springboard/node/$gid/group" : 'springboard/groups/settings';
      $permissions = l($text, "$path/permissions", array('query' => array('destination' => 'admin/springboard/groups/settings/roles')));
    }
    if ($bundle != 'springboard_group') {
      if (!$default_access && !in_array($name, array(OG_ANONYMOUS_ROLE, OG_AUTHENTICATED_ROLE))) {
        $rows[] = array(
          check_plain($name),
          l(t('edit role'), "$path/role/$rid/edit"),
          $permissions,
        );
      }
      else {
        $rows[] = array(check_plain($name), t('locked'), $permissions);
      }
    }
    else {
      if (!$default_access && !in_array($name, array(OG_ANONYMOUS_ROLE, OG_AUTHENTICATED_ROLE))) {
        $rows[] = array(
          check_plain($name),
          l(t('edit role'), "$path/role/$rid/edit", array('query' => array('destination' => 'admin/springboard/groups/settings/roles'))),
          $permissions,
        );
      }
      else {
        $rows[] = array(check_plain($name), t('locked'), $permissions);
      }
    }
  }

  $rows[] = array(array('data' => drupal_render($form['name']) . drupal_render($form['add']), 'colspan' => 4, 'class' => 'edit-name'));

  hide($form['actions']);
  $output = drupal_render_children($form);
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= render($form['actions']);

  return $output;
}

/**
 * Implements hook_form_alter().
 */
function springboard_group_form_og_ui_admin_new_role_alter(&$form, &$form_state) {
  if ($form['bundle']['#value'] == 'springboard_group') {
    $form['add']['#submit'][] = 'springboard_group_form_og_ui_admin_new_role_submit';
  }
}

/**
 * Redirect to our custom roles page.
 */
function springboard_group_form_og_ui_admin_new_role_submit($form, &$form_state) {
  $form_state['redirect'] = array('admin/springboard/groups/settings/roles');
}