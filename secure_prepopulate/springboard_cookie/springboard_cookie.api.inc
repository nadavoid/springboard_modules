<?php

/**
 * @file
 * Provides documentation of available springboard_cookie hooks.
 */

/**
 * Check if the cookie should save profile values to the cookie.
 *
 * @form_state
 *   The submitted form state.
 *
 * @return bool
 *   Whether to store profile data in the cookie, return FALSE to prevent.
 */
function hook_springboard_cookie_should_store_profile_values($form_state) {
  return TRUE;
}
