<?php

/**
 * @file
 * Designation component functions.
 */

/**
 * Default values for designation form.
 *
 * @return array
 *   Component defaults.
 */
function _webform_defaults_designations() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'extra' => array(
      'field_prefix' => '',
      'field_suffix' => '',
    ),
  );
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_designations($component) {
  $form = array();
  $form['edit_notice']['#markup'] = '<div>' . t('The settings for this field are controlled by Fundraiser.') . '</div>';
  $form['display']['#access'] = FALSE;
  return $form;
}

/**
 * Implements _webform_submit_component().
 *
 * Convert submitted keys and values into serialized, saveable data.
 */
function _webform_submit_designations($component, $values) {
  $return = array();
  foreach ($values as $key => $submitted_value) {
    $return[] = serialize(array($key => $submitted_value));
  }
  return $return;
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_designations($component, $value = NULL, $filter = TRUE) {

  $element = array(
    '#type' => 'item',
    '#title' => $filter ? _webform_filter_xss($component['name']) : $component['name'],
    '#weight' => $component['weight'],
    '#attributes' => array('class' => array('webform-component-designations')),
    '#title_display' => 'none',
    '#webform_component' => $component,
    '#theme' => 'fundraiser_designations_fund_groups_component',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'fundraiser_designations') . '/js/fundraiser_designations.js'),
      'css' => array(drupal_get_path('module', 'fundraiser_designations') . '/css/designation-client-form.css'),
    ),
  );

  $node = isset($component['nid']) ? node_load($component['nid']) : NULL;

  // Get the one-time and/or recurring price sets to be displayed as radios.
  $amounts = fundraiser_designations_get_form_display_amounts($node);

  if (fundraiser_designations_is_designation_type($node->type) && !empty($node->designation_group_ids)) {

    // Load the fund group entities.
    $groups = fundraiser_fund_group_load_multiple($node->designation_group_ids);
    // Arrange the groups by weight set by draggableviews.
    $groups_by_weight = [];
    foreach ($groups as $key => $group) {
      $groups_by_weight[$group->data['weight'] . $key] = $group;
    }
    ksort($groups_by_weight);

    // Build the groups for display.
    foreach ($groups_by_weight as $group) {

      $group_wrapper = entity_metadata_wrapper('fundraiser_fund_group', $group);
      $funds = $group_wrapper->field_ffg_fd_reference->value();
      uasort($funds, function ($a, $b) {
        return strcmp($a->title, $b->title);
      });
      $num = count($funds);

      // Groups with multiple funds referenced get a select field, a "group of
      // one" fund doesn't.
      if ($num > 1) {
        _webform_render_funds_multiple($element, $node, $group, $funds, $amounts, $groups_by_weight);
      }
      elseif ($num == 1) {
        _webform_render_funds_single($element, $node, $group, $funds, $amounts, $groups_by_weight);
      }
    }
  }
  return $element;
}

/**
 * Render the component when it has multiple funds.
 *
 * @param array $element
 *   The webform component.
 * @param object $node
 *   The node.
 * @param object $group
 *   The fund group.
 * @param object $funds
 *   The funds in the fund group.
 * @param array $amounts
 *   The one-time and/or recurring price sets.
 * @param array $groups_by_weight
 *   The fund groups on this form.
 */
function _webform_render_funds_multiple(&$element, $node, $group, $funds, $amounts, $groups_by_weight) {
  // @todo these two functions can be combined.
  $options = array();
  $options[] = 'Select';
  foreach ($funds as $fund) {
    if ($fund->status == 1) {
      $options[$fund->product_id] = check_plain($fund->title);
    }
  }

  $group_wrapper = entity_metadata_wrapper('fundraiser_fund_group', $group);
  $id = $group_wrapper->fundraiser_fund_group_id->value();

  $item = array();
  $item['#id'] = 'group-item-' . $id;
  $item['#extra']['group_id'] = $id;
  $item['#extra']['fund_type'] = 'multiple';
  $item['#extra']['position'] = _webform_render_funds_get_position($groups_by_weight, $id);
  $item['#type'] = 'item';
  $element[$id] = $item;

  $child = array();
  $child['#type'] = 'select';
  $child['#id'] = 'funds-select-' . $id;
  $child['#extra']['group_id'] = $id;
  $child['#options'] = $options;
  $child['#title'] = '';
  $element[$id]['funds_select_' . $id] = $child;

  _fundraiser_designations_build_child_elements($id, $element, $amounts, $group, $node);
}

/**
 * Render the component when it has a single fund.
 *
 * @param array $element
 *   The webform component.
 * @param object $node
 *   The node.
 * @param object $group
 *   The fund group.
 * @param object $funds
 *   The funds in the fund group.
 * @param array $amounts
 *   The one-time and/or recurring price sets.
 * @param array $groups_by_weight
 *   The fund groups on this form.
 */
function _webform_render_funds_single(&$element, $node, $group, $funds, $amounts, $groups_by_weight) {
  $group_wrapper = entity_metadata_wrapper('fundraiser_fund_group', $group);
  $id = $group_wrapper->fundraiser_fund_group_id->value();
  $item = array();
  $item['#id'] = 'group-item-' . $id;
  $item['#extra']['group_id'] = $id;
  $item['#extra']['fund_type'] = 'single';
  $item['#extra']['fund_id'] = $funds[0]->product_id;
  $item['#attributes'] = array('class' => $funds[0]->product_id);

  $item['#extra']['position'] = _webform_render_funds_get_position($groups_by_weight, $id);
  $item['#type'] = 'item';
  $element[$id] = $item;

  $child = array();
  $child['#type'] = 'item';
  $child['#id'] = 'funds-placeholder-' . $id;
  $child['#extra']['group_id'] = $id;
  $child['#title'] = check_plain($funds[0]->title);
  $element[$id]['funds_placeholder_' . $id] = $child;

  _fundraiser_designations_build_child_elements($id, $element, $amounts, $group, $node);

}

/**
 * Figure out which groups are first and last to add CSS classes.
 *
 * @param array $groups_by_weight
 *   An array of fund groups.
 * @param int $id
 *   A group id.
 *
 * @return string
 *   A css class.
 */
function _webform_render_funds_get_position($groups_by_weight, $id) {
  reset($groups_by_weight);
  $first_key = key($groups_by_weight);
  end($groups_by_weight);
  $last_key = key($groups_by_weight);
  if ($groups_by_weight[$first_key]->fundraiser_fund_group_id == $id) {
    return 'desigation-group-first';
  }
  if ($groups_by_weight[$last_key]->fundraiser_fund_group_id == $id) {
    return 'desigation-group-last';
  }
  return 'desigation-group-middle';
}


/**
 * Adds the quantity and amount forms to the group.
 *
 * @param int $id
 *   A group entity id.
 * @param array $element
 *   The parent webform component.
 * @param array $amounts
 *   The one-time and/or recurring price sets.
 */
function _fundraiser_designations_build_child_elements($id, &$element, $amounts, $group, $node) {
  if (!empty($group->fd_overrides) && !empty($group->fd_overrides['status'])) {
    $amounts = fundraiser_designations_get_group_display_amounts($group, $node);
  }

  // Determine whether the designation group overrides the form's default ask
  // amounts, then determine the default ask option for both one-time and
  // recurring based on either the form's settings or
  // the designation group's settings, as appropriate.
  $default_default_amount = false;
  $recurring_default_amount = false;
  // Check the designation group for overrides.
  if (isset($group->fd_overrides['price_set']) && $group->fd_overrides['status'] == '1') {
    $price_set = unserialize($group->fd_overrides['price_set']);
    // If there are overrides, determine the default option for
    // one-time ask amounts.
    if (isset($price_set['default'])) {
      foreach ($price_set['default'] as $price) {
        if ($price['default_amount'] === 1) {
          $default_default_amount = $price['amount'];
        }
      }
    }

    // If there are overrides, determine the default option for
    // recurring ask amounts.
    if (isset($price_set['recurring'])) {
      foreach ($price_set['recurring'] as $price) {
        // Find the one that is set to default.
        if ($price['default_amount'] === 1) {
          $recurring_default_amount = $price['amount'];
        }
      }
    }
  }
  // If there are not overrides and the parent node has default options set,
  // use those values.
  elseif (isset($node->default_amount) || isset($node->recurring_default_amount)) {
    // Parent node one-time default option.
    if (isset($node->default_amount)){
      $default_default_amount = intval($node->default_amount);
    }
    // Parent node recurring default option.
    if (isset($node->recurring_default_amount)) {
      $recurring_default_amount = intval($node->recurring_default_amount);
    }
  }

  $donation_amounts = [];
  if (!empty($amounts['default'])) {
    foreach ($amounts['default'] as $key => $amount) {
      $donation_amounts[$amount[0]] = check_plain($amount[1]);
    }
    $child = array();
    $child['#type'] = 'radios';
    $child['#id'] = 'default-amounts-' . $id;
    $child['#extra']['group_id'] = $id;
    $child['#options'] = $donation_amounts;
    $child['#title'] = '';
    // Insert the default one-time option if applicable.
    if($default_default_amount) {
      $child['#default_value'] = $default_default_amount;
    }

    $element[$id]['default_amounts_' . $id] = $child;
  }

  $recurring_donation_amounts = [];
  if (!empty($amounts['recurring'])) {
    foreach ($amounts['recurring'] as $key => $amount) {
      $recurring_donation_amounts[$amount[0]] = check_plain($amount[1]);
    }
    $child = array();
    $child['#type'] = 'radios';
    $child['#id'] = 'recurring-amounts-' . $id;
    $child['#extra']['group_id'] = $id;
    $child['#options'] = $recurring_donation_amounts;
    $child['#title'] = '';
    // Insert the default recurring option if applicable.
    if($recurring_default_amount) {
      $child['#default_value'] = $recurring_default_amount;
    }
    $element[$id]['recurring_amounts_' . $id] = $child;
  }
  $group_wrapper = entity_metadata_wrapper('fundraiser_fund_group', $group);
  $other_amount = $group_wrapper->field_ffg_other_amt[0]->value();
  if (!empty($other_amount)) {
    $currency = fundraiser_get_currency_from_node($node);
    $child = array();
    $child['#type'] = 'textfield';
    $child['#id'] = 'fd-other-' . $id;
    $child['#extra']['group_id'] = $id;
    $child['#title'] = 'Other amount';
    $child['#description'] = 'Minimum payment ' . $currency['symbol'] . number_format($node->minimum_donation_amount, 2) . '.';
    $element[$id]['fd_other_' . $id] = $child;
  }

  $quantity = $group_wrapper->field_ffg_quantity[0]->value();

  if (!empty($quantity)) {
    $child = array();
    $child['#type'] = 'select';
    $child['#id'] = 'funds-quant-select-' . $id;
    $child['#extra']['group_id'] = $id;
    $child['#options'] = array(1 => 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    $child['#title'] = 'Quantity';
    $element[$id]['funds_quant_select_' . $id] = $child;
  }

  $child = array();
  $child['#type'] = 'button';
  $child['#id'] = 'add-' . $id;
  $child['#extra']['group_id'] = $id;
  $child['#value'] = 'Add';
  $element[$id]['add_' . $id] = $child;
}

/**
 * Theme the form elements into a table.
 */
function theme_fundraiser_designations_fund_groups_component($variables) {
  $element = $variables['element'];
  $output = '';
  $node = menu_get_object();
  if (empty(element_children($element)) && (user_access("edit any {$node->type} content") || user_access("edit own {$node->type} content") || user_access("administer fund groups"))) {
    $output = t('No designation groups have been created yet. You can !create now.', array('!create' => l('create one', 'node/' . arg(1) . '/designations/add/fundraiser_fund_group_default')));
  }
  $x = 0;

  foreach (element_children($element) as $id) {
    $build = array();
    if (isset($element[$id]['#extra']['group_id'])) {

      $group_wrapper = entity_metadata_wrapper('fundraiser_fund_group', entity_load_single('fundraiser_fund_group', $element[$id]['#extra']['group_id']));
      $description = $group_wrapper->field_ffg_description->value();
      $image = $group_wrapper->field_ffg_image->value();
      if (!empty($image)) {
        $image['path'] = file_create_url($image['uri']);
      }
      $build['content'] = array(

        '#prefix' => '<div class="designation-group-wrapper ' . $element[$id]['#extra']['position'] . '" id="designation-group-' . $element[$id]['#extra']['group_id'] . '">',
        '#suffix' => '</div>',

        'fund_group_title' => array(
          '#prefix' => '<div class="designation-group-title"><h4>',
          '#suffix' => '</h4></div>',
          '#markup' => check_plain($group_wrapper->name->value()),
          '#weight' => -9,
        ),

        'fund_group_image' => array(
          '#type' => 'markup',
          '#markup' => !empty($image) ? theme('image', $image) : '',
          '#prefix' => '<div class="designation-group-image">',
          '#suffix' => '</div>',
          '#weight' => -8,
        ),

        'fund_group_description' => array(
          '#type' => 'markup',
          '#markup' => check_markup($description['value'], $description['format']),
          '#prefix' => '<div class="designation-group-description">',
          '#suffix' => '</div>',
          '#weight' => -7,
        ),

        'fund_group_table' => array(
          '#type' => 'markup',
          '#markup' => fundraiser_designations_selection_table($element[$id]),
          '#prefix' => '<div class="designation-group-funds-table-wrapper">',
          '#suffix' => '</div>',
          '#weight' => -6,
        ),
      );

      // Add subheader.
      if ($x === 1) {
        $pid = $element['#webform_component']['pid'];
        $nid = $element['#webform_component']['nid'];
        $component = webform_menu_component_load($pid, $nid, NULL);
        if (!empty($component['extra']['subheader'])) {
          $build['content']['fund_group_subheader'] = array(
            '#prefix' => '<div class="designation-group-subheader"><h3>',
            '#suffix' => '</h3></div>',
            '#markup' => check_plain($component['extra']['subheader']),
            '#weight' => -9,
          );
        }
      }
      $x++;

      $output .= drupal_render($build);
    }
  }

  return $output;
}

/**
 * Build a table display for each fund group.
 *
 * @param array $element
 *   The fund group component.
 *
 * @return string
 *   The rendered table.
 */
function fundraiser_designations_selection_table($element) {
  $row[] = array(
    'data' => array(
      array(
        'data' => drupal_render($element),
        'align' => 'right',
      ),
    ),
    'class' => array('group-row-' . $element['#extra']['group_id']),
  );

  if (isset($element['#extra']['fund_id'])) {
    $row[0]['data-placeholder-fund-id'] = $element['#extra']['fund_id'];
  }

  $table = array(
    '#theme' => 'table',
    '#header' => array(),
    '#rows' => $row,
    '#attributes' => array(
      'class' => array('designation-group-funds-table'),
    ),
  );
  return drupal_render($table);
}

/**
 * Get the one-time and/or recurring price sets for this form.
 *
 * @param object $node
 *   The donation form node.
 *
 * @return array
 *   An array of prices arranged by type.
 */
function fundraiser_designations_get_form_display_amounts($node) {
  $price_sets = &drupal_static(__FUNCTION__);
  if (!isset($price_sets)) {
    $recurring = $node->recurring_setting;
    $recurring_amounts = $node->recurring_donation_amounts;
    $donation_amounts = $node->donation_amounts;
    $dual_ask = $node->recurring_dual_ask_amounts;
    $default_amount = $node->default_amount;
    $recurring_default_amount = $node->recurring_default_amount;
    $dynamic = FALSE;

    if (module_exists('dynamic_gift_strings')) {
      if (isset($_SESSION['dynamic_gift_string_values'][$node->nid]) && is_array($_SESSION['dynamic_gift_string_values'][$node->nid])) {
        $dynamic = TRUE;
        $gs = $_SESSION['dynamic_gift_string_values'][$node->nid];
        if (!empty($gs['amounts'])) {
          $dynamic = TRUE;
          $gs_amounts = _dynamic_gift_strings_parse_amounts($gs['amounts'], $node->show_other_amount, $node->nid);
          if ($gs_amounts) {
            $donation_amounts = [];
            $default_amount = $_SESSION['dynamic_gift_string_values'][$node->nid]['default'];
            foreach ($gs_amounts as $plain => $c_coded) {
              if (is_numeric($plain)) {
                $donation_amounts[] = array($plain, $c_coded);
              }
            }
          }
        }
        if (!empty($gs['recurring_amounts'])) {
          $gs_recurring_amounts = _dynamic_gift_strings_parse_amounts($gs['recurring_amounts'], $node->recurring_show_other_amount, $node->nid);
          if ($gs_recurring_amounts) {
            $recurring_amounts = [];
            $recurring_default_amount = $_SESSION['dynamic_gift_string_values'][$node->nid]['recurring_default'];
            foreach ($gs_recurring_amounts as $plain => $c_coded) {
              if (is_numeric($plain)) {
                $recurring_amounts[] = array($plain, $c_coded);
              }
            }
          }
        }
      }
    }

    $price_sets = [];
    $price_sets['default_amount'] = $default_amount;
    $price_sets['recurring_default_amount'] = $recurring_default_amount;
    $price_sets['dynamic'] = $dynamic;

    switch ($recurring) {
      case 'never':
        $price_sets['default'] = $donation_amounts;
        break;

      case 'user_choice':
        if (!empty($dual_ask)) {
          $price_sets['default'] = $donation_amounts;
          $price_sets['recurring'] = $recurring_amounts;
        }
        else {
          $price_sets['default'] = $donation_amounts;
        }
        break;

      case 'always':
        $price_sets['recurring'] = $recurring_amounts;
        break;
    }
  }
  return $price_sets;
}

/**
 * Get the override display amounts for a group.
 *
 * @param object $group
 *   The fund group to apply price sets to.
 * @param object $node
 *   The node which owns the fund group.
 *
 * @return array
 *   A price set.
 */
function fundraiser_designations_get_group_display_amounts($group, $node) {

  // Get the default donation amounts for the form.
  // Used if price override is enabled, but no overries defined. (i.e., when
  // switching to dual ask on alredy overriden forms).
  $node_form_amounts = fundraiser_designations_get_form_display_amounts($node);
  // Get the group override ask amounts.
  $override_price_set = unserialize($group->fd_overrides['price_set']);
  // Build the price set array.
  $price_sets = [];
  $donation_amounts = [];

  // Get default override.
  if (isset($override_price_set['default'])) {
    foreach ($override_price_set['default'] as $key => $price) {
      if (!empty($price['default_amount'])) {
        $price_sets['default']['default_amount'] = $price['default_amount'];
      }
      $donation_amounts['default'][$key][] = check_plain($price['amount']);
      $donation_amounts['default'][$key][] = check_plain($price['label']);
    }
  }
  else {
    // If no default override, use the donation form amounts.
    $donation_amounts['default'] = $node_form_amounts['default'];
  }

  // Get the recurring overrides.
  if (isset($override_price_set['recurring'])) {
    foreach ($override_price_set['recurring'] as $key => $price) {
      if (!empty($price['default_amount'])) {
        $price_sets['recurring']['recurring_default_amount'] = $price['default_amount'];
      }
      $donation_amounts['recurring'][$key][] = check_plain($price['amount']);
      $donation_amounts['recurring'][$key][] = check_plain($price['label']);

    }
  }
  elseif(isset($node_form_amounts['recurring'])) {
    // If no recurring override get the donation form default.
    $donation_amounts['recurring'] = $node_form_amounts['recurring'];
  }

  $recurring = $node->recurring_setting;
  $dual_ask = $node->recurring_dual_ask_amounts;

  // Set the override amounts based on form configuraiton.
  switch ($recurring) {
    case 'never':
      $price_sets['default'] = $donation_amounts['default'];
      break;

    case 'user_choice':
      if (!empty($dual_ask)) {
        $price_sets['default'] = $donation_amounts['default'];
        if (!empty($donation_amounts['recurring'])) {
          $price_sets['recurring'] = $donation_amounts['recurring'];
        }
        else {
          $price_sets['recurring'] = $donation_amounts['default'];
        }
      }
      else {
        $price_sets['default'] = $donation_amounts['default'];
      }
      break;

    case 'always':
      $price_sets['recurring'] = $donation_amounts['recurring'];
      break;
  }

  return $price_sets;
}
