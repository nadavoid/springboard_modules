<?php

/**
 * @file
 * Utility functions for Springboard data warehouse.
 */

/**
 * Get all entity properties by way of a wrapper.
 *
 * @param object $wrapper
 *   A wrapper object used to key off to get key value formatted entities.
 *
 * @return array
 *   Entity values
 */
function springboard_dw_get_all_entity_properties($wrapper) {

  $values = array();

  foreach ($wrapper->getPropertyInfo() as $key => $val) {
    $values[$key] = $wrapper->$key->value();
  }

  return $values;
}

/**
 * Get the webform submission dated keyed according to submission.
 *
 * @param int $nid
 *   The node ID to retrieve webform submission data by.
 * @param int $sid
 *   The submission ID to retrieve webform submission data by.
 *
 * @return array
 *   The keyed submission data array.
 */
function springboard_dw_webform_submission_data_keyed($nid, $sid) {

  $data = array();
  $node = node_load($nid);

  module_load_include('inc', 'webform', 'includes/webform.submissions');
  $submission = webform_get_submission($nid, $sid);

  foreach ($node->webform['components'] as $key => $component) {
    if (isset($submission->data[$key])) {
      $data[$component['form_key']] = $submission->data[$key];
    }
  }

  return $data;
}

/**
 * Gets array of donation-enabled content types.
 */
function springboard_dw_get_donation_types() {
  $list = array();
  $types = fundraiser_get_donation_types();
  foreach ($types as $key => $value) {
    if ($value) {
      $list[] = $key;
    }
  }
  return $list;
}

/**
 * Examines sync map items and determines their corresponding
 * data warehouse types.
 *
 * @param string $module
 *   The name of the handling module.
 * @param string $delta
 *   The delta of the handling module.
 * @param string $object_type
 *   The type of Salesforce object.
 *
 * @return string
 */
function springboard_dw_convert_sf_type($module, $delta, $object_type) {

  if ($delta == 'user') {
    return 'Contact';
  }

  if ($delta == 'sba_message_action' || $delta == 'sba_social_action' || $delta == 'springboard_petition') {
    return 'Form';
  }

  if ($delta == 'donation') {
    return 'Donation';
  }

  if ($object_type == 'sb_Actions_Taken__c') {
    return 'Action';
  }

  // If it's not one of the above types we're not going to deal with it.
  return FALSE;
}


/**
 * Load the timestamp of a submission record from {webform_submissions}.
 *
 * @param int $sid
 *   Webform submission id.
 *
 * @return int
 *   Returns submitted timestamp for a given webform submission.
 */
function springboard_dw_get_submission_timestamp($sid) {
  $result = db_query('
    SELECT
      submitted
    FROM {webform_submissions}
    WHERE
      sid = :sid
    LIMIT 1
  ', array(':sid' => $sid));
  return $result->fetchField();
}
