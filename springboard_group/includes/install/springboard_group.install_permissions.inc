<?php

/**
 * Install the Affilaite editor role and default
 * permissions for groups.
 */
function _springboard_group_install_permissions() {

  $roles = array('Affiliate editor');
  foreach ($roles as $role) {
    if (!user_role_load_by_name($role)) {
      $role = new stdClass();
      $role->name = 'Affiliate editor';
      user_role_save($role);
    }
  }

  $permissions = _springboard_group_install_permissions_default();
  $roles = [];
  $roles[] = user_role_load_by_name('Administrator');
  $roles[] = user_role_load_by_name('Springboard administrator');
  $roles[] = user_role_load_by_name('Springboard editor');
  $roles[] = user_role_load_by_name('Affiliate editor');


  foreach ($roles as $role) {
    $current_perms = user_role_permissions($role);
    foreach ($permissions as $name => $perm) {
      if (isset($perm['roles'][$role->name]) && empty($current_perms[$name])) {
        user_role_grant_permissions($role->rid, array($name));
      }
    }
  }
}

/**
 * @return array
 */
function _springboard_group_install_permissions_default() {

  $permissions = array();

  if (module_exists('contextual')) {
    $permissions['access contextual links'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }

  if (module_exists('draggableviews')) {
    $permissions['access draggableviews'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }

  if (module_exists('filter')) {
    $permissions['use text format filtered_html'] = array(
      'name' => 'use text format filtered_html',
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
    $permissions['use text format full_html'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }
  if (module_exists('toolbar')) {
    $permissions['access toolbar'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }
  if (module_exists('path')) {
    $permissions['create url aliases'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }

  if (module_exists('springboard')) {
    $permissions['access springboard dashboard'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }

  if (module_exists('webform_user')) {
    $permissions['configure webform emails'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
    $permissions['configure webform settings'] = array(
      'roles' => array(
        'Affiliate editor' => 'Affiliate editor',
      ),
    );
  }

  $permissions['access springboard groups dashboard'] = array(
    'roles' => array(
      'Affiliate editor' => 'Affiliate editor',
      'Administrator' => 'Administrator',
      'Springboard admin' => 'Springboard admin',
      'Springboard editor' => 'Springboard editor',
    ),
  );

  $permissions['configure springboard groups'] = array(
    'roles' => array(
      'Administrator' => 'Administrator',
    ),
  );
  return $permissions;
}
