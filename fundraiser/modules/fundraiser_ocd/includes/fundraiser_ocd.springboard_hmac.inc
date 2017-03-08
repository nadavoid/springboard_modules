<?php
/**
 * @file
 * Springboard HMAC hook implementations for Springboard One Click Donate.
 */

/**
 * Create HMAC token link for one-click donations.
 *
 * @param array $payload
 *   The payload values.
 *
 * @return string
 *   The one-click donation url.
 */
function fundraiser_ocd_create_link($payload, $rollback = FALSE) {
  $link = '';

  $time = time() + (60 * 60 * 24 * 30);
  $values = array(
    $payload['uid'],
    $time,
    'ocd',
    $payload['card_id'],
  );

  $payload_string = implode('.', $values);

  $payload_string = base64_encode($payload_string);
  $key = springboard_hmac_get_key();
  $hmac = springboard_hmac_create_hmac($values, $key);

  $link = url('hmac/' . $payload_string . '/' . $hmac, array('absolute' => TRUE));

  return $link;
}

/**
 * Implements hook_springboard_hmac_allowed_actions().
 */
function fundraiser_ocd_springboard_hmac_allowed_actions() {
  return 'ocd';
}

/**
 * Implements hook_springboard_hmac_success().
 *
 * Redirect to the donation form if hmac validated.
 */
function fundraiser_ocd_springboard_hmac_success($payload) {
  if ($payload['action'] != 'ocd') {
    return;
  }

  // Make payload variable easier to work with.
  $payload = _fundraiser_ocd_hmac_mapping($payload);

  // First, verify the uid in the payload matches what's stored in the cookie,
  // if it doesn't we can just stop right here.
  if (!fundraiser_ocd_user_is_opted_in($payload['uid'])) {
    // TODO: What is the ideal user experience for someone who has lost
    // their cookie? Let's log this occurence but should we take them to the
    // form anyway and let them donate the old fashioned way?
    watchdog('ocd', 'One-click donate attempted but opt-in could not be verified for user %uid and card_id %card_id.', array('%uid' => $payload['uid'], '%card_id' => $payload['card_id']), WATCHDOG_WARNING);
    // Redirect to the default default node ID if it's set, otherwise return.
    $goto_nid = fundraiser_ocd_get_default_form_for_gateway('default');
    if ($goto_nid) {
      drupal_goto('node/' . $goto_nid);
    }
    return;
  }

  // Figure out where we're going. Could be the default or a url param override.
  $goto_nid = _fundraiser_ocd_get_nid($payload);

  $user = _fundraiser_ocd_springboard_hmac_check_user($goto_nid, $payload);

  // Ensure that the card ID is valid and belongs to the user.
  $card = commerce_cardonfile_load($payload['card_id']);
  // @todo Determine if we should use commerce_cardonfile_access in this
  // condition instead.
  if (!$card || $card->uid != $user->uid && !$card_to_be_updated) {
    // Redirect to the default default form, or return if not set.
    $goto_nid = fundraiser_ocd_get_default_form_for_gateway('default');
    if ($goto_nid) {
      drupal_goto('node/' . $goto_nid);
    }
    return;
  }

  // Otherwise save the card ID to a session and redirect to the calculated form
  // ID.
  $_SESSION['springboard_hmac']['ocd']['card_id'] = $payload['card_id'];
  drupal_goto('node/' . $goto_nid);
}

/**
 * Helper function to check if a user is logged in or if their account exists.
 */
function _fundraiser_ocd_springboard_hmac_check_user($goto_nid, $payload) {
  global $user;
  // If the user isn't logged in, attempt to load them. We're doing this to see
  // if the user account still exists.
  if (!user_is_logged_in()) {
    // Prep the log-in.
    $existing_user = user_load($payload['uid']);

    // If it's an existing user, we'll be able to log them in after
    // verification.
    if ($existing_user) {
      // Overwrite the anonymous user prior to login.
      $user = $existing_user;
      $form_state = array('uid' => $payload['uid']);

      // Execute the log-in.
      user_login_finalize($form_state);
    }
    else {
      // User no longer exists.
      $cookie = springboard_cookie_get_cookie();
      if (isset($cookie['profile']) && !empty($cookie['profile'])) {
        $_SESSION['secure_prepopulate_values'][$goto_nid] = $cookie['profile'];
        // Since the rest of the validation involves card data, which the user
        // will need to re-enter, just redirect them now.
        drupal_goto('node/' . $goto_nid);
      }
    }
  }

  return $user;
}

/**
 * Return the node ID of the fundraiser for an HMAC token.
 *
 * If the ID is not available in the token, return the default for the gateway.
 * If the default is not available, return an error.
 *
 * @param array $payload
 *   The payload array.
 *
 * @return int|void
 *   The fundraiser ID or an invokation of `springboard_hmac_failure`.
 */
function _fundraiser_ocd_get_nid($payload) {
  if (!isset($payload['card_id'])) {
    $payload = _fundraiser_ocd_hmac_mapping($payload);
  }

  $parameters = drupal_get_query_parameters();
  // Look up the gateway's instance id based on the stored card.
  $card_on_file = commerce_cardonfile_load($payload['card_id']);
  $gateway_id = $card_on_file->instance_id;

  // If a form_id was passed, let's try to use it first.
  if (isset($parameters['form_id']) && is_numeric($parameters['form_id']) &&
    fundraiser_ocd_form_has_gateway($parameters['form_id'], $gateway_id)) {
    return $parameters['form_id'];
  }

  // Get the default nid for the gateway instance.
  if ($nid = fundraiser_ocd_get_default_form_for_gateway($gateway_id)) {
    return $nid;
  }

  // Return the default donation form nid (DD) if set.
  if ($nid = fundraiser_ocd_get_default_form_for_gateway('default')) {
    return $nid;
  }

  // We were unable to determine which form to use w/ the given gateway.
  return FALSE;
}

/**
 * Returns the configured default form for a given payment gateway instance.
 *
 * @param string $gateway_id
 *   The gateway instance identifier.
 *
 * @return int|FALSE
 *   The nid of the default form for the gateway instance or FALSE.
 */
function fundraiser_ocd_get_default_form_for_gateway($gateway_id) {
  $default_data = variable_get(fundraiser_ocd_DEFAULT_FORM_PREFIX . $gateway_id, array());
  if (!empty($default_data)) {
    $default_data = unserialize($default_data);
    return $default_data['nid'];
  }

  return FALSE;
}

/**
 * Determines if a given gateway instance is configured on a form.
 *
 * @param int $nid
 *   The nid of the form being checked.
 * @param string $gateway_id
 *   The gateway instance identifier.
 *
 * @return bool
 *   If the gateway is configured on the form, true. Otherwise false.
 */
function fundraiser_ocd_form_has_gateway($nid, $gateway_id) {
  $gateways = db_query('SELECT gateways FROM {fundraiser} WHERE nid = :nid', array(':nid' => $nid))->fetchField();
  if ($gateways) {
    $gateways = unserialize($gateways);
    foreach ($gateways as $gateway) {
      if (!empty($gateway['id']) && $gateway['id'] == $gateway_id) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/**
 * Determines if a given user is opted into one-click donate.
 *
 * Uses the Springboard cookie to see if the given uid matches
 * the value stored in the browser's cookie and that the one-click
 * donate opt-in data is set.
 *
 * @param int $uid
 *   The uid to verify.
 *
 * @return bool
 *   True if the user is opted into one-click donate. Otherwise false.
 */
function fundraiser_ocd_user_is_opted_in($uid) {
  $cookie = springboard_cookie_get_cookie();
  return isset($cookie['uid']) && $cookie['uid'] == $uid;
}

/**
 * Implements hook_springboard_hmac_failure().
 *
 * Redirect to the donation form if hmac not validated.
 */
function fundraiser_ocd_springboard_hmac_failure($reason, $payload) {
  if (empty($payload['action']) || $payload['action'] != 'ocd') {
    return;
  }

  $payload = _fundraiser_ocd_hmac_mapping($payload);
  $nid = _fundraiser_ocd_get_nid($payload);

  // If the payload didn't validate any number of things could have happened,
  // therefore let's fallback to the donation form.
  //@TODO: Add a drupal_set_message() here to indicate an HMAC error is why this isn't the exact experience expected?
  drupal_goto('node/' . $nid);
}

/**
 * Implements hook_springboard_hmac_verify_payload().
 */
function fundraiser_ocd_springboard_hmac_verify_payload($payload) {
  if ($payload['action'] != 'ocd') {
    return;
  }

  // Require a non-logged-in user to match the Springboard cookie.
  if ($failure = springboard_hmac_verify_user_cookie($payload)) {
    return $failure;
  }

  $payload = _fundraiser_ocd_hmac_mapping($payload);
  // If the card id isn't available or doesn't belong to the user we can't
  // continue.
  if (!isset($payload['card_id']) || !is_numeric($payload['card_id'])) {
    return t('Unable to perform one-click donate because stored card information was not available in the payload.');
  }
  elseif (!_fundraiser_ocd_card_belongs_to_user($payload['uid'], $payload['card_id'])) {
    return t('Unable to perform one-click donate because stored card id %card_id does not belong to user %uid.', array('%card_id' => $payload['card_id'], '%uid' => $payload['uid']));
  }

  // Since we want to have the ability of redirecting the user to a default form
  // most of the actual verification is done in the
  // `hook_springboard_hmac_success` hook.
  return NULL;
}

/**
 * Helper function to get a mapping of hmac values for one-click donations.
 *
 * Needed since by default the array isn't mapped by key but instead by index.
 *
 * @return array
 *   A map of hmac types to their corresponding keys.
 */
function _fundraiser_ocd_hmac_mapping($payload) {
  if (empty($payload)) {
    return array();
  }

  return array(
    'uid' => $payload['uid'],
    'card_id' => $payload[0],
  );
}

/**
 * Helper function to determine if the given card belongs to a user.
 *
 * @param int $uid
 *   The user id to check.
 * @param int $card_id
 *   The card id to check.
 *
 * @return bool
 *   True if the card belongs to the user. Otherwise false.
 */
function _fundraiser_ocd_card_belongs_to_user($uid, $card_id) {
  $card_owner = db_query("SELECT uid FROM {commerce_cardonfile} WHERE card_id = :card_id", array(':card_id' => $card_id))->fetchField();
  return $card_owner == $uid;
}
