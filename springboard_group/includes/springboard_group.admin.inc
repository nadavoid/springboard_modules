<?php
/**
 * @file
 * Config functions for Springboard Group.
 */

/**
 * Create the groups admin page.
 *
 * @return array
 *   A page render array.
 */
function springboard_group_settings_page() {

  $build = array();
  $build['content'] = array(
    'page_description' => array(
      '#id' => 'springboard-advocacy-settings-page',
      '#type' => 'markup',
      '#markup' => '<p>' . t('Choose which content types and entity types are eligible to be grouped.') . '</p>',
      '#prefix' => '<div class="springboard-groups-settings-page">',
      '#suffix' => '</div>',
    ),
    'springboard_group_node_settings_form' => drupal_get_form('springboard_group_node_settings_form'),
    'springboard_group_entity_settings_form' => drupal_get_form('springboard_group_entity_settings_form'),
  );
  return $build;
}

/**
 * Build a form listing group-eligible node types.
 *
 * @return array
 *   The eligible node type form.
 */
function springboard_group_node_settings_form() {
  $form = array();

  $form['#attached']['css'][] = drupal_get_path('module', 'springboard_group') . '/css/springboard-og-admin.css';

  $form['springboard_group_node'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content types'),
    '#collapsible' => TRUE,
    '#description' => t('Make these content types eligible for grouping'),
    '#tree' => TRUE,
  );

  // Get all node types. Remove the springboard_group node type from the list.
  $content_type_options = [];
  $options = node_type_get_types();
  usort($options, function($a, $b) {
    return strcmp($a->name, $b->name);
  });
  foreach ($options as $key => $type) {
    if ($type->type != 'springboard_group') {
      $content_type_options[$type->type] = $type->name;
    }
  }

  // Get the already groups-enabled node types.
  $groups_enabled_types = springboard_group_get_grouped_types();
  $node_defaults = !empty($groups_enabled_types['node']) ? array_keys($groups_enabled_types['node']) : [];

  $disallowed_content_types = array(
    'p2p_campaign',
    'p2p_campaign_landing',
    'p2p_category',
    'p2p_personal_campaign',
  );

  foreach ($content_type_options as $type => $name) {
    // Require springboard group advocacy module be enabled before allowing
    // groupable actions.
    if (!module_exists('springboard_group_advocacy') && in_array($type, array('sba_message_action', 'sba_social_action'))) {
      continue;
    }
    if (in_array($type, $disallowed_content_types)) {
      continue;
    }
    $form['springboard_group_node'][$type] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('group-content-wrapper')),
    );

    $form['springboard_group_node'][$type]['name'] = array(
      '#type' => 'item',
      '#title' => $name,
    );

    $form['springboard_group_node'][$type][$type] = array(
      '#title' => 'Enable grouping',
      '#type' => 'checkbox',
      '#default_value' => in_array($type, $node_defaults),
    );

    // Add the multiple selection option.
    $disallowed_multiple = array('sba_message_action', 'sba_social_action');
    $donation_types = array_keys(array_filter(fundraiser_get_donation_types()));
    $disallowed_multiple = array_merge($disallowed_multiple, $donation_types);
    if (!in_array($type, $disallowed_multiple)) {
      $form['springboard_group_node'][$type]['og_' . $type . '_multiple'] = array(
        '#title' => 'Allow multiple group selections.',
        '#type' => 'checkbox',
        '#default_value' => variable_get('og_' . $type . '_multiple', 0),
        '#attributes' => array('class' => array('multiple')),
        '#states' => array(
          'visible' => array(
            ':input[name*="' . $type . ']"]' => array('checked' => TRUE),
          ),
        ),
      );
    }

    // Disable the checkboxes on the already enabled node types and
    // add a link to remove group field.
    if (in_array($type, $node_defaults)) {
      $og_field = $groups_enabled_types['node'][$type][0];
      $form['springboard_group_node'][$type][$type]['#disabled'] = TRUE;
      $form['springboard_group_node'][$type][$type]['#description'] = l(t('Disable grouping'), "admin/config/group/fields/node/$type/" . $og_field . "/delete", array('query' => array('destination' => 'admin/springboard/groups/settings')));
    }
  }

  $form['springboard_group_node']['submit_node'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('springboard_group_settings_form_submit'),
  );

  return $form;
}

/**
 * Build a form listing group-eligible entites.
 *
 * @return array
 *   The eligible entity and node type forms.
 */
function springboard_group_entity_settings_form() {

  $form = array();
  $form['#attached']['css'][] = drupal_get_path('module', 'springboard_group') . '/css/springboard-og-admin.css';

  // Get an array of fieldable entities.
  $fieldable_entities = array();
  foreach (entity_get_info() as $entity_type => $entity_info) {
    if (empty($entity_info['fieldable'])) {
      continue;
    }
    foreach ($entity_info['bundles'] as $bundle_name => $bundle) {
      // Prefix the bundle name with the entity type.
      $entity_name = check_plain("$entity_info[label] ($entity_type)");
      $fieldable_entities[$entity_name][$entity_type . ':' . $bundle_name] = filter_xss($bundle['label']);
    }
  }

  // Filter out some entities we don't want in the list.
  $entity_type_options = [];
  foreach ($fieldable_entities as $key => $type) {
    foreach ($type as $info => $name) {
      if (strpos($info, 'node') === FALSE && strpos($info, 'og_') === FALSE && strpos($info, 'line_item') === FALSE) {
        $entity_type_options[$info] = $name . ' (' . substr($info, 0, strpos($info, ':')) . ')';
      }
    }
  }

  asort($entity_type_options);
  $form['springboard_group_entities'] = array(
    '#type' => 'fieldset',
    '#title' => t('Entity types'),
    '#collapsible' => TRUE,
    '#description' => t('Make these entities eligible for grouping'),
    '#tree' => TRUE,
  );

  // Get the already-enabled entities.
  $groups_enabled_types = springboard_group_get_grouped_types();

  $entity_defaults = !empty($groups_enabled_types) ? $groups_enabled_types : [];
  $group_enabled_entities = [];
  foreach ($entity_defaults as $entity_type => $bundle_group) {
    if ($entity_type != 'node') {
      foreach ($bundle_group as $bundle => $field) {
        $group_enabled_entities[] = $entity_type . ':' . $bundle;
      }
    }
  }
  $disallowed = array(
    'sba_message',
    'sba_message:sba_message_action_message',
    'sba_message:sba_social_action_twitter_message',
    'taxonomy_term',
    'taxonomy_term:tags',
    'taxonomy_term:sba_legislative_issues',
    'taxonomy_term:sba_organization_issues',
    'commerce_cardonfile:commerce_cardonfile',
    'commerce_customer_profile:billing',
    'commerce_product',
    'commerce_product:product',
    'commerce_product:fundraiser_ticket',
  );

  foreach ($entity_type_options as $type => $name) {
    if (!in_array($type, $disallowed) && !in_array(substr($type, 0, strpos($type, ':')), $disallowed)) {
      $name = str_replace('Rules configuration', 'Payment Gateways', $name);
      $form['springboard_group_entities'][$type] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('group-content-wrapper')),
      );
      $form['springboard_group_entities'][$type]['name'] = array(
        '#type' => 'item',
        '#title' => $name,
      );
      $form['springboard_group_entities'][$type][$type] = array(
        '#title' => 'Enable grouping',
        '#type' => 'checkbox',
        '#default_value' => in_array($type, $group_enabled_entities),
      );
    }

    $disallowed_multiple = array(
      'commerce_cardonfile:commerce_cardonfile',
      'commerce_customer_profile:billing',
      'commerce_order:commerce_order',
      'user:user',
    );

    if (!in_array($type, $disallowed) && !in_array($type, $disallowed_multiple)) {
      list(, $bundle) = explode(':', $type);
      $form['springboard_group_entities'][$type]['og_' . $type . '_multiple'] = array(
        '#title' => 'Allow multiple group selections.',
        '#type' => 'checkbox',
        '#default_value' => variable_get('og_' . $bundle . '_multiple', 0),
        '#attributes' => array('class' => array('multiple')),
        '#states' => array(
          'visible' => array(
            ':input[name*="' . $type . ']"]' => array('checked' => TRUE),
          ),
        ),
      );
    }
  }

  // Disable the checkboxes on types that are already enabled.
  // Add a link to remove the group field.
  foreach ($entity_defaults as $entity_type => $bundle_group) {
    if ($entity_type != 'node') {
      foreach ($bundle_group as $bundle => $field) {
        $form['springboard_group_entities'][$entity_type . ':' . $bundle][$entity_type . ':' . $bundle]['#disabled'] = TRUE;
        $form['springboard_group_entities'][$entity_type . ':' . $bundle][$entity_type . ':' . $bundle]['#description'] = l(t('Disable grouping'), "admin/config/group/fields/" . $entity_type . "/" . $bundle . "/" . $field[0] . "/delete", array('query' => array('destination' => 'admin/springboard/groups/settings')));
      }
    }
  }

  $form['springboard_group_entities']['actions'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('springboard_group_entities_settings_form_submit'),
  );

  return $form;
}

/**
 * Submits the groupable node type form.
 */
function springboard_group_settings_form_submit($form, $form_state) {
  foreach ($form_state['values']['springboard_group_node'] as $node_type => $status) {
    if ($node_type == 'submit_node') {
      continue;
    }
    if (!empty($status[$node_type]) && !empty($form_state['input']['springboard_group_node'][$node_type])) {
      springboard_group_add_group_reference_field($node_type, 'node');
      springboard_group_add_group_activate_views_alter($node_type);
    }
    if (isset($form_state['values']['springboard_group_node'][$node_type]['og_' . $node_type . '_multiple'])) {
      variable_set('og_' . $node_type . '_multiple', $form_state['values']['springboard_group_node'][$node_type]['og_' . $node_type . '_multiple']);
    }
  }
}

/**
 * Submits the groupable entity type form.
 */
function springboard_group_entities_settings_form_submit($form, $form_state) {
  foreach ($form_state['values']['springboard_group_entities'] as $key => $status) {
    if (in_array($key, array('', 'actions'))) {
      continue;
    }
    list($entity_type, $bundle) = explode(':', $key);
    if (!empty($status[$key]) && !empty($form_state['input']['springboard_group_entities'][$key])) {
      springboard_group_add_group_reference_field($bundle, $entity_type);
      springboard_group_add_group_activate_views_alter($bundle);

    }

    if (isset($form_state['values']['springboard_group_entities'][$key]['og_' . $key . '_multiple'])) {
      variable_set('og_' . $bundle . '_multiple', $form_state['values']['springboard_group_entities'][$key]['og_' . $key . '_multiple']);
    }
  }
}

/**
 * Creates the og reference field.
 *
 * @param string $bundle
 *   The bundle name.
 * @param string $type
 *   The entity type name.
 */
function springboard_group_add_group_reference_field($bundle, $type) {
  if ($bundle != 'user') {
    $og_field = og_fields_info('og_group_ref');
    $og_field['instance']['settings']['behaviors']['og_widget']['admin']['widget_type'] = 'options_select';
    $og_field['field']['settings']['handler'] = 'og_springboard_og';
    $og_field['field']['settings']['handler_settings']['target_bundles'] = array('springboard_group' => 'springboard_group');
    $og_field['field']['settings']['handler_settings']['sort'] = array(
      'direction' => 'ASC',
      'property' => 'title',
      'type' => 'property',
    );
    $og_field['instance']['display']['default']['type'] = 'hidden';
    $og_field['instance']['display']['default']['label'] = 'hidden';
    $og_field['instance']['display']['teaser']['type'] = 'hidden';
    $og_field['instance']['display']['teaser']['label'] = 'hidden';

    // Enable Entity reference prepopulate.
    $og_field['instance']['settings']['behaviors']['prepopulate'] = array(
      'status' => TRUE,
      'action' => 'none',
      'fallback' => 'none',
      'skip_perm' => FALSE,
    );
    og_create_field('og_group_ref_' . substr($bundle, 0, 19), $type, $bundle, $og_field);
  }
  else {
    // Users get an ordinary entity reference field.
    $entity_reference_field['field']['module'] = 'entityreference';
    $entity_reference_field['field']['type'] = 'entityreference';
    $entity_reference_field['field']['cardinality'] = -1;
    $entity_reference_field['field']['settings']['handler_settings']['target_bundles'] = array('springboard_group' => 'springboard_group');
    $entity_reference_field['field']['settings']['handler'] = 'base';
    $entity_reference_field['field']['settings']['handler_settings']['behaviors']['views-select-list']['status'] = 0;
    $entity_reference_field['field']['settings']['handler_settings']['sort'] = array(
      'direction' => 'ASC',
      'property' => 'title',
      'type' => 'property',
    );
    $entity_reference_field['field']['settings']['handler_settings']['target_bundles'] = array(
      'springboard_group' => 'springboard_group',
    );

    $entity_reference_field['instance']['widget'] = array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_select',
      'weight' => 29,
    );
    $entity_reference_field['instance']['display']['default']['type'] = 'hidden';
    $entity_reference_field['instance']['display']['default']['label'] = 'hidden';
    $entity_reference_field['instance']['display']['teaser']['type'] = 'hidden';
    $entity_reference_field['instance']['display']['teaser']['label'] = 'hidden';
    $entity_reference_field['instance']['label'] = t('Contact owners');
    $entity_reference_field['instance']['description'] = t('The groups this contact is owned by.');
    // Enable Entity reference prepopulate.
    $entity_reference_field['instance']['settings']['behaviors']['prepopulate'] = array(
      'status' => 0,
    );

    $field = field_info_field('user_sb_group_ref');
    // Allow overriding the field name.
    $entity_reference_field['field']['field_name'] = 'user_sb_group_ref';
    if (empty($field)) {
      $field = field_create_field($entity_reference_field['field']);
    }

    $instance = field_info_instance($type, 'user_sb_group_ref', $bundle);
    if (empty($instance)) {
      $instance = $entity_reference_field['instance'];
      $instance += array(
        'field_name' => 'user_sb_group_ref',
        'bundle' => $bundle,
        'entity_type' => $type,
      );

      field_create_instance($instance);
    }
  }
}

/**
 * Advanced config options for groups.
 *
 * @return mixed
 *   A system settings form.
 */
function springboard_group_advanced_settings() {
  $form = array();

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => 'Advanced options',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['advanced']['access'] = array(
    '#type' => 'fieldset',
    '#title' => 'Access control',
    '#description' => '',
    '#collapsible' => TRUE,
  );

  $form['advanced']['access']['og_node_access_strict'] = array(
    '#type' => 'checkbox',
    '#title' => t('Strict node access permissions'),
    '#default_value' => variable_get('og_node_access_strict', TRUE),
  );
  $form['advanced']['access']['help'] = array(
    '#markup' => '<p>' . t('Allow groups permissions to override Springboard Permissions.'),
  );

  $form['advanced']['roles'] = array(
    '#type' => 'fieldset',
    '#title' => 'Roles',
    '#description' => '',
    '#collapsible' => TRUE,
  );

  $form['advanced']['roles']['springboard_group_show_non_member'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show the "non member" role on the groups permissions page.'),
    '#default_value' => variable_get('springboard_group_show_non_member', NULL),
  );

  $form['advanced']['debug'] = array(
    '#type' => 'fieldset',
    '#title' => 'Debug group context.',
    '#description' => t('Show a message with current group context on every group-relevant page.'),
    '#collapsible' => TRUE,
  );

  $form['advanced']['debug']['springboard_group_debug_context'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show debug message'),
    '#default_value' => variable_get('springboard_group_debug_context', NULL),
  );

  $form['altered_views'] = array(
    '#type' => 'fieldset',
    '#title' => 'Groups-enabled Springboard views',
    '#description' => t("Restrict results for these views based on group context. Enabling this option will cause the view query to be altered, filtering the result based on the value of the groups context selector. For users who only belong to one group, the context selector is not displayed and the group ID is pulled directly from the user session."),
    '#collapsible' => TRUE,
  );

  // Enable disabling/enabling of the views_query_alter hook per form.
  // Useful for debugging.
  $alter = array(
    'sbv_forms',
    'sbv_page_wrappers',
    'sbv_assets',
    'webform_ab_tests',
    'sba_actions',
    'sbv_email_templates',
    'commerce_orders',
    'sbv_donations',
    'sbv_contacts',
  );

  $all_views = views_get_all_views();
  $omit = array('og_all_user_group_content', 'og_nodes', 'springboard_group', 'donation_forms');
  foreach ($all_views as $key => $view) {
    if (in_array($view->name, $alter)) {
      unset($all_views[$key]);
    }
    if (in_array($view->name, $omit)) {
      unset($all_views[$key]);
    }
    if ($view->base_table != 'node') {
      unset($all_views[$key]);
    }
  }
  $optional_views = array();
  foreach ($all_views as $key => $opt) {
    $optional_views[$key] = $opt->human_name;
  }

  $form['altered_views']['standard'] = array(
    '#type' => 'fieldset',
    '#title' => 'Standard views',
    '#description' => t("This views are part of Springboard Core, and are automatically integrated with groups when their associated contents types are made groupable. You can disable them here if you need to debug."),
    '#collapsible' => TRUE,
  );

  $form['altered_views']['standard']['group_restricted_views'] = array(
    '#type' => 'checkboxes',
    '#options' => drupal_map_assoc($alter),
    '#default_value' => variable_get('group_restricted_views', array()),
  );

  $form['altered_views']['standard']['force'] = array(
    '#type' => 'fieldset',
    '#title' => '',
    '#description' => "",
    '#collapsible' => FALSE,
  );

  $form['altered_views']['standard']['force']['force_group_restricted_views'] = array(
    '#type' => 'checkbox',
    '#title' => t('Force group ownership field on overidden views.'),
    '#description' => t("By default the group ownership field won't display on overridden springboard core views. Check this box to force the field to display. Alternatively, you can add the springboard group field manually in the views UI, which would give you control of its order in the columns"),
    '#default_value' => variable_get('force_group_restricted_views', 0),
  );

  $form['altered_views']['optional'] = array(
    '#type' => 'fieldset',
    '#title' => 'Optional views',
    '#description' => t("These views are not automatically integrated with groups, and require configuration to become groups-aware. Check the checkbox below to insert the group ownership field into the view display and to limit the results for the view by group. Then add the display's menu path to the textarea below."),
    '#collapsible' => TRUE,
  );

  $form['altered_views']['optional']['optional_group_restricted_views'] = array(
    '#type' => 'checkboxes',
    '#options' => $optional_views,
    '#default_value' => variable_get('optional_group_restricted_views', array()),
  );

  $form['altered_views']['paths'] = array(
    '#type' => 'fieldset',
    '#title' => 'Additional group context filter display paths.',
    '#description' => '',
    '#collapsible' => TRUE,
  );

  $form['altered_views']['paths']['optional_group_menu_paths'] = array(
    '#title' => t('Add additional paths, one per line.'),
    '#description' => t('By default, the groups context filter automatically appears on a pre-determined set of pages. To add additional pages, enter the paths, one per line, where you want the groups context filter to display. These paths should match the paths for the optional views you selected above. Example: admin/content/donation-forms'),
    '#type' => 'textarea',
    '#default_value' => variable_get('optional_group_menu_paths', ''),
  );

  return system_settings_form($form);
}

/**
 * Activate views hook_query_alter for views whose node types are grouped.
 *
 * @param string $type
 *   The node type.
 */
function springboard_group_add_group_activate_views_alter($type) {
  $restricted = variable_get('group_restricted_views', array());
  switch ($type) {
    case 'page_wrapper':
      $restricted = array_merge($restricted, drupal_map_assoc(array('sbv_page_wrappers', 'sbv_assets')));
      break;

    case 'email_wrapper':
      $restricted = array_merge($restricted, drupal_map_assoc(array('sbv_email_templates', 'sbv_assets')));
      break;

    case 'webform_ab':
      $restricted = array_merge($restricted, drupal_map_assoc(array('webform_ab_tests')));
      break;

    case 'user':
      $restricted = array_merge($restricted, drupal_map_assoc(array('sbv_contacts')));
      break;

    case 'commerce_order':
      $restricted = array_merge($restricted, drupal_map_assoc(array('commerce_orders')));
      $restricted = array_merge($restricted, drupal_map_assoc(array('sbv_donations')));
      break;

    case 'springboard_petition':
    case 'sba_message_action':
    case 'sba_social_action':
      $restricted = array_merge($restricted, drupal_map_assoc(array('sba_actions')));
      break;

    default:
      if (fundraiser_is_donation_type($type)) {
        $restricted = array_merge($restricted, drupal_map_assoc(array('sbv_forms')));
      }
      if (in_array($type, webform_variable_get('webform_node_types'))) {
        $restricted = array_merge($restricted, drupal_map_assoc(array('sbv_forms')));
      }
  }

  variable_set('group_restricted_views', $restricted);
}
