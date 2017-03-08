<?php

/**
 * @file
 * Helper functions for the secure prepopulate component.
 */

/**
 * Insert new components on node/add.
 *
 * Called from secure_prepopulate_node_insert();
 *
 * @param $node
 */
function secure_prepopulate_insert_components($node) {
  // Add the component to the Webform.
  // Don't insert field if cloning.
  if (!isset($node->clone_from_original_nid)) {
    module_load_include('inc', 'webform', 'includes/webform.components');

    $field = array(
      'nid' => $node->nid,
      'form_key' => 'secure_prepop_autofilled',
      'pid' => 0,
      'name' => t('Autofilled'),
      'type' => 'hidden',
      'value' => 0,
      'weight' => 13,
      'email' => 1,
      '#extra' => array(
        'description' => '',
        'hidden_type' => 'hidden',
      ),
    );
    webform_component_insert($field);
  }
}
