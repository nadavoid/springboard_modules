<?php
/**
 * @file
 * springboard_p2p.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function springboard_p2p_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'register for springboard p2p'.
  $permissions['register for springboard p2p'] = array(
    'name' => 'register for springboard p2p',
    'roles' => array(
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'springboard_p2p',
  );

  // Exported permission: 'create p2p_personal_campaign content'.
  $permissions['create p2p_personal_campaign content'] = array(
    'name' => 'create p2p_personal_campaign content',
    'roles' => array(
      'Springboard P2P campaigner' => 'Springboard P2P campaigner',
    ),
    'module' => 'node',
  );

  // Exported permission: 'edit own p2p_personal_campaign content'.
  $permissions['edit own p2p_personal_campaign content'] = array(
    'name' => 'edit own p2p_personal_campaign content',
    'roles' => array(
      'Springboard P2P campaigner' => 'Springboard P2P campaigner',
    ),
    'module' => 'node',
  );


  return $permissions;
}
