<?php
/**
 * @file
 * Rule definitions and hooks for Springboard Group.
 */

/**
 * Implements hook_rules_configuration().
 *
 * A rule which enables bulk adding of users to a group. Uses native OG rule
 * "og_group_content_add".
 */
function springboard_group_default_rules_configuration() {
  $items['rules_springboard_group_bulk_add'] = entity_import('rules_config', '{ "rules_springboard_group_bulk_add" : {
    "LABEL" : "add_og_membership",
    "PLUGIN" : "action set",
    "REQUIRES" : [ "og", "rules" ],
    "ACCESS_EXPOSED" : "1",
    "USES VARIABLES" : { "user" : { "label" : "user", "type" : "user" } },
    "ACTION SET" : [
      { "og_group_content_add" : { "entity" : [ "user" ], "group" : [ "site:springboard-group-context--node" ] } },
      { "drupal_message" : { "message" : "User [user:name] added to group" } }
    ]
  }
}');

  return $items;
}

function springboard_group_entity_property_info() {
  $info = array();
  foreach (og_get_all_group_entity() as $entity_type => $label) {
    $params = array('@label' => $label);
    $properties = &$info['site']['properties'];
    $properties['springboard_group_context__' . $entity_type] = array(
      'label' => t('Current OG group of @label entity type', $params),
      'description' => t('The @label that is an OG group from context, if exists.', $params),
      'getter callback' => 'springboard_group_get_properties',
      'type' => $entity_type,
    );
  }

  return $info;
}

function springboard_group_get_properties($data = array(), array $options, $name, $type) {
  // Get the entity type form the property name.
  $group_type = str_replace('springboard_group_context__', '', $name);
  if (og_is_group($group_type, arg(1))) {
    return entity_load_single($group_type, arg(1));
  }
}

function springboard_group_rules_config_are_grouped($fields = FALSE) {
  $group_enabled_field = &drupal_static(__FUNCTION__ . $fields);
  if (!isset($group_enabled_field)) {
    $group_enabled_field = og_get_group_audience_fields('rules_config', 'rules_config', 'node', 'springboard_group');
  }
  if (!$fields) {
    return (bool) $group_enabled_field;
  }
  else {
    return $group_enabled_field;
  }
}


/**
 * Implements hook_rules_config_access().
 *
 * Adds access control checks to commerce payment rules.
 */
function springboard_group_rules_config_access($op, $rules_config = NULL, $account = NULL) {
  // Access control on already-existing rules.
  if ($op == 'update') {
    $group = og_get_entity_groups('rules_config', $rules_config);
    if (!empty($group)) {
      // Access control on rules that already have a group.
      $gid = reset($group['node']);
      $group_context = springboard_group_get_group_context();
      if ($roles = og_get_user_roles('node', $gid)) {
        $perms = og_role_permissions($roles);
        $role_permissions = array_keys(array_filter(reset($perms)));
        if ($group_context == $gid && in_array('edit payment method in springboard group', $role_permissions)) {
          return TRUE;
        }
      }
      if ($group_context != $gid && !user_access('administer group')) {
        return FALSE;
      }
    }
    else {
      // Access control on rules that don't have a group yet.
      if ($rules_config->module == 'commerce_payment') {
        $fields = springboard_group_rules_config_are_grouped();
        if (!empty($fields)) {
          if ((user_access('administer rules') || user_access('administer payment methods')) && !user_access('administer group')) {
            return FALSE;
          }
        }
      }
    }
  }
}

/**
 * Filter the payment methods overview table by group context.
 *
 * Called from hook_page_alter().
 */
function springboard_group_rules_payment_rules_overview__page_alter(&$page) {
  if (!empty($page['content']['system_main']['#page_callback']) &&
    $page['content']['system_main']['#page_callback'] == 'commerce_payment_ui_admin_page'
  ) {
    $role_permissions = array();
    $context_gid = springboard_group_get_group_context();
    if (!empty($context_gid) && $context_gid != '_none' && $context_gid != '_all') {
      $roles = og_get_user_roles('node', $context_gid);
      $perms = og_role_permissions($roles);
      $role_permissions = array_keys(array_filter(reset($perms)));
    }
    foreach (array('enabled', 'disabled') as $key) {
      foreach ($page['content']['system_main'][$key]['rules']['#rows'] as $row_key => &$row) {
        $machine_name = (str_replace('Machine name: ', '', $row[0]['data']['description']['settings']['machine_name']['#markup']));
        $rule = rules_config_load($machine_name);
        if (!empty($rule)) {
          $group = og_get_entity_groups('rules_config', $rule);
          if (!empty($group)) {
            $gid = reset($group['node']);
          }
          else {
            $gid = !is_numeric($context_gid) ? $context_gid : NULL;
          }
          if ($context_gid == '_all') {
            $all_gids = og_get_all_group('node');
            if (!in_array($gid, $all_gids)) {
              $invalid = TRUE;
            }
            else {
              $invalid = FALSE;
            }
          }
          else {
            $invalid = $context_gid != $gid;
          }
          if ($invalid || (!user_access('administer group') && !in_array('edit payment method in springboard group', $role_permissions))) {
            unset($page['content']['system_main'][$key]['rules']['#rows'][$row_key]);
          }
        }
      }
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Contortions required to get field api working on rules config.
 */
function springboard_group_form_rules_ui_add_element_alter(&$form, &$form_state, $form_id) {
  if (arg(1) == 'ajax') {
    $rule_name = !empty($form_state['parent_element']->name) ? $form_state['parent_element']->name : '';
    if (!empty($rule_name)) {
      $rule = rules_config_load($rule_name);
      if (empty($rule)) {
        // It's a custom UI managed rule.
        $rule = rules_config_load('rules_' . $rule_name);
      }
      if (!empty($rule)) {
        //springboard_group_rules_add_group_widget($form, $form_state, $rule);
      }
    }
  }
}


/**
 * Implements hook_form_FORMID_alter().
 *
 * Contortions required to get field api working on rules config.
 */
function springboard_group_form_rules_ui_edit_element_alter(&$form, &$form_state, $form_id) {
  // We need the rule name to proceed.
  $rule_name = arg(6) != 'edit' ? arg(6) : arg(5);
  if (!empty($rule_name)) {
    $rule = rules_config_load($rule_name);
  }
  if (empty($rule) && !empty($rule_name)) {
    // It's a custom UI managed rule.
    $rule = rules_config_load('rules_' . $rule_name);
  }
  if (empty($rule)) {
    // Ajax.
    $rule = !empty($form_state['build_info']['args'][0]->name) ? rules_config_load($form_state['build_info']['args'][0]->name) : FALSE;
  }
  if (!empty($rule) && empty($form['events'])) {
    springboard_group_rules_add_group_widget($form, $form_state, $rule);
  }
}

/**
 * Implements hook_form_FORMID_alter().
 *
 * Contortions required to get field api working on rules config.
 */
function springboard_group_form_rules_ui_form_edit_rules_config_alter(&$form, &$form_state, $form_id) {
  // We need the rule name to proceed.
  if (empty($form['settings']['name']['#default_value'])) {
    return;
  }

  $rule_name = $form['settings']['name']['#default_value'];
  $rule = rules_config_load($rule_name);
  if (empty($rule) && !empty($rule_name)) {
    // It's a custom UI managed rule.
    $rule = rules_config_load('rules_' . $rule_name);
  }
  if (!empty($rule)) {
    //springboard_group_rules_add_group_widget($form, $form_state, $rule);
  }

}

/**
 * Add the group widget to the rules config form.
 */
function springboard_group_rules_add_group_widget(&$form, &$form_state, $rule) {

  // Only add the OG field to commerce rules.
  if (!empty($rule) && $rule->module == 'commerce_payment' || in_array('commerce_payment', $rule->dependencies)) {

    // Rules config entities are fieldable but rules module does not attach
    // fields, yet...
    field_attach_form('rules_config', $rule, $form, $form_state);

    // Check that this entity has a groups field.
    $childs = element_children($form);
    $grouped = FALSE;
    foreach ($childs as $child) {
      if (strpos($child, 'og_group_ref') !== FALSE) {
        $grouped = TRUE;
      }
    }

    // No groups field, do not proceed.
    if (!$grouped) {
      return;
    }

    // Add the rule entity to the form so we can use it in validate and submit.
    $form['rule_entity'] = array(
      '#type' => 'value',
      '#value' => $rule,
    );

    // Convert the multiple select widget to a single selection widget.
    // See springboard_group_field_widget_form_alter() and
    // springboard_group_set_single_selector() for details about
    // why this is necessary.
    $fields = springboard_group_rules_config_are_grouped(TRUE);

    if (!empty($fields)) {
      springboard_group_set_single_selector($form, $fields);

      // Add ajax callback to the groups field so we can validate newly
      // selected groups.
      $language = $form['og_group_ref_' . 'rules_config']['#language'];
      $form['og_group_ref_' . 'rules_config'][$language][0]['default']['#ajax'] = array(
        'callback' => 'springboard_group_og_group_field_ajax_callback',
      );

      // Set up the validation modal window parameters.
      springboard_group_prepare_validation_modal($form, $form_state);

      // Add the validation handlers to the form.
      springboard_group_load_ajax_validation_handlers($form, $form_state);
    }

    $form['#validate'][] = 'springboard_group_validate_payment';
    $form['#submit'][] = 'springboard_group_submit_payment';
  }
}


/**
 * Run field attach validation.
 */
function springboard_group_validate_payment($form, &$form_state) {
  field_attach_form_validate('rules_config', $form_state['values']['rule_entity'], $form, $form_state);
//  if (empty($target) && !user_access('administer group')) {
//    form_set_error('og_group_ref_rules_config', t('You must select a group.'));
//  }
}

/**
 * Run field attach save functions.
 */
function springboard_group_submit_payment($form, &$form_state) {
  $rule = $form_state['values']['rule_entity'];
  field_attach_submit('rules_config', $rule, $form, $form_state);
  field_attach_insert('rules_config', $rule);
}

function springboard_group_rules_config_form__validate(&$form, &$form_state, $callback = NULL) {
  if ($callback != 'springboard_group_og_group_field_ajax_callback') {
    return;
  }

  if (!empty($form['settings']['name']['#default_value'])) {
    $rule_name = $form['settings']['name']['#default_value'];
    if (!empty($rule_name)) {
      $rule = rules_config_load($rule_name);
    }

    if (empty($rule) && !empty($rule_name)) {
      // It's a custom UI managed rule.
      $rule = rules_config_load('rules_' . $rule_name);
    }
  }

  if (empty($rule)) {
    // Ajax.
    $rule = !empty($form_state['build_info']['args'][0]->name) ? rules_config_load($form_state['build_info']['args'][0]->name) : FALSE;
  }

  $this_rule_old_gids = og_get_entity_groups('rules_config', $rule, array(), 'og_group_ref_rules_config');
  if (isset($this_rule_old_gids['node'])) {
    $this_rule_old_gids = $this_rule_old_gids['node'];
  }
  else {
    $this_rule_old_gids = [];

  }
  $this_rule_new_gids = springboard_group_get_gids_from_element($form, $form_state);


  // Select all records so we can search inside the serizalied gateways field.
  $donation_forms = db_query("SELECT nid, gateways FROM {fundraiser}");
  if ($donation_forms) {
    // Compile list of forms to use when building the message to the user.
    $matching_donation_forms = array();
    while ($donation_form = $donation_forms->fetchAssoc()) {
      $gateways = unserialize($donation_form['gateways']);
      foreach ($gateways as $gateway) {
        if (!empty($gateway['id'])) {
          if (strpos($gateway['id'], $rule->name) && $gateway['status'] == 1) {
            $matching_donation_forms[] = $donation_form['nid'];
          }
        }
      }
    }
    if (!empty($matching_donation_forms)) {
      $matching_nodes = db_select('node', 'n')
        ->condition('nid', $matching_donation_forms, 'IN')
        ->fields('n', array('nid', 'title'))
        ->execute();
      while ($matching_node = $matching_nodes->fetchAssoc()) {
        // Get the wrapped node so we can get its groups.
        // @todo can we fold this into the common functions that wrappers use?
        $donation_form_node = node_load($matching_node['nid']);
        $donation_form_node_groups = og_get_entity_groups('node', $donation_form_node);
        if (!empty($this_rule_old_gids)) {
          if ((!empty($this_rule_new_gids) && !empty($donation_form_node_groups['node']))) {
            $valid = array_intersect($this_rule_new_gids, $donation_form_node_groups['node']);
          }
          elseif (empty($this_rule_new_gids) && !empty($donation_form_node_groups['node'])) {
            $valid = FALSE;
          }
          elseif (!empty($this_rule_new_gids) && empty($donation_form_node_groups['node'])) {
            $valid = FALSE;
          }
          else {
            $valid = TRUE;
          }
          if (empty($valid)) {
            $in_use[] = (array) $donation_form_node;
          }
        }
        else {

          // This rule had no group to begin with.
          $valid = !empty($donation_form_node_groups['node']);
          // If there's no match we have a problem.
          if (empty($valid) && !empty($this_rule_new_gids)) {
            $in_use[] = (array) $donation_form_node;
          }
        }
      }
    }
  }
  if (!empty($in_use)) {
    $count = count($in_use);
    $format = format_plural($count, 'node', 'nodes');
    $uses = format_plural($count, 'uses', 'use');
    $rows = [];
    foreach ($in_use as $wrapped) {
      $node = node_load($wrapped['nid']);
      $rows[] = l(check_plain($node->title), 'node/' . $wrapped['nid'], array('attributes' => array('target' => '_blank')));
    }
    $message = t('@count @format belonging to an incompatible group @use this rule.', array(
      '@count' => $count,
      '@format' => $format,
      '@use' => $uses,
    ));
    if ($count < 50) {
      $message .= theme('item_list', array('items' => $rows));
    }
    $form_state['group_ajax_messages'][] = $message;
  }
}


/**
 * Sub-hook to springboard_group_og_permissions().
 */
function springboard_group__rules_config__og_permissions(&$items) {
  $items['edit payment method in springboard group'] = array(
    'title' => t('Edit payment methods in Springboard Group'),
    'description' => t("Edit a group's payments methods."),
    'roles' => array(OG_AUTHENTICATED_ROLE, OG_ADMINISTRATOR_ROLE),
    'default role' => array(),
    'module' => 'commerce_payment',
  );
}
