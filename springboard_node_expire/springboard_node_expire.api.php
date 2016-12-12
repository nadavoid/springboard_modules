<?php
/**
 * @file
 * API documentation for springboard_node_expire.
 */

/**
 * Allow other modules to alter the expiration behavior of a fundraiser.
 *
 * @param array $data
 *   An array of information about the expiration action:
 *     - expires [bool] Whether the fundraiser has expired.
 *     - action [integer] The action to take if the fundraiser has expired. Can
 *       be either FUNDRAISER_EXPIRATION_ACTION_MESSAGE or
 *       FUNDRAISER_EXPIRATION_ACTION_REDIRECT.
 *     - message [array] An array of information about the message to display.
 *       - message [string] The message to display
 *       - format [string] The format of the message, defaults to filtered_html.
 *     - redirect [string] The URL to redirect to.
 *     - access [boolean] Whether the current user has access to update the
 *       fundraiser.
 */
function hook_springboard_node_expire_alter($data) {
}

/**
 * Allow other modules to alter the node view before expiration message appears.
 *
 * @param object &$node
 *   The node variable passed to hook_node_view().
 *
 * @see hook_node_view()
 */
function hook_springboard_node_expire_node_view_alter(&$node) {
}

/**
 * Allow other modules to act before the expiration behavior of a fundraiser.
 *
 * @param array $node
 *   The fundraiser node object.
 * @param array $data
 *   An array of information about the expiration action. More information can
 *   be found in the function documentation for
 *   hook_fundraiser_expiration_alter().
 */
function hook_springboard_node_expire_action_before($node, $data) {
}

/**
 * Allow other modules to act after the expiration behavior of a fundraiser.
 *
 * @param array $node
 *   The fundraiser node object.
 * @param array $data
 *   An array of information about the expiration action. More information can
 *   be found in the function documentation for
 *   hook_fundraiser_expiration_alter().
 */
function hook_springboard_node_expire_action_after($node, $data) {
}
