<?php
/**
 * @file
 * Node hooks for Springboard Group Advocacy.
 */

/**
 * Implements hook_node_insert().
 */
function springboard_group_advocacy_node_insert($node) {
  if ($node->type == 'springboard_group') {
    // Save the advocacy group subscription config.
    springboard_group_advocacy_save($node);
  }
}

/**
 * Implements hook_node_update().
 */
function springboard_group_advocacy_node_update($node) {
  if ($node->type == 'springboard_group') {
    // Save the advocacy group subscription config.
    springboard_group_advocacy_save($node);
  }
}

/**
 * Implements hook_node_load().
 *
 * Items needed for node cloning.
 */
function springboard_group_advocacy_node_load($nodes, $types) {
  if (!in_array('springboard_group', $types)) {
    return;
  }
  foreach ($nodes as $nid => $node) {
    if ($node->type == 'springboard_group') {
      $subscription = springboard_group_advocacy_get_saved_group_subscription($nid);
      if (!empty($subscription)) {
        $node->master_subscription_type = $subscription['master_subscription_type'];
        $node->group_subscription_type = $subscription['group_subscription_type'];
        $node->allowed_states = !empty($subscription['allowed_states']) && is_array(unserialize($subscription['allowed_states'])) ? unserialize($subscription['allowed_states']) : [];
      }
    }
  }
}

/**
 * Save the group's advocacy subscription data.
 *
 * @param object $node
 *   The Springboard Group node.
 */
function springboard_group_advocacy_save($node) {
  if (user_access('administer springboard advocacy') || user_access('administer advocacy group subscription')) {
    // Is there an existing subscription record?
    $exists = db_query('select gid from {sb_og_advocacy} where gid = :gid', (array(':gid' => $node->nid)))->fetchField();

    // If the subscription level permits state subscriptions, save them;
    // otherwise, remove any previously existing state data.
    $allowed_states = $node->group_subscription_type != 'federal-and-states' && $node->group_subscription_type != 'states-only' ? array_filter($node->allowed_states) : [];

    $data = array(
      'gid' => $node->nid,
      "allowed_states" => $allowed_states,
      'master_subscription_type' => $node->master_subscription_type,
      'group_subscription_type' => $node->group_subscription_type,
    );

    if (!empty($exists)) {
      drupal_write_record('sb_og_advocacy', $data, array('gid'));
    }
    else {
      drupal_write_record('sb_og_advocacy', $data);
    }
  }
}
