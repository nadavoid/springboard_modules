<?php

/**
 * @file
 * Definition of springboard_target_views_access.
 */

/**
 * Access plugin that provides permission-based access control.
 *
 * @ingroup views_access_plugins
 */
class springboard_target_views_access extends views_plugin_access {
  /**
   * Determine if the current user has access or not.
   */
  function access($account) {

    return springboard_target_target_access($this->options['perm']);
  }

  function get_access_callback() {
    return array('springboard_target_target_access', array($this->options['perm']));
  }

  function summary_title() {
    $permissions = module_invoke_all('permission');
    if (isset($permissions[$this->options['perm']])) {
      return $permissions[$this->options['perm']]['title'];
    }

    return t($this->options['perm']);
  }


  function option_definition() {
    $options = parent::option_definition();
    $options['perm'] = array('default' => 'access content');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $perms = array();
    $module_info = system_get_info('module');

    // Get list of permissions
      $permissions = module_invoke('springboard_target', 'permission');
      foreach ($permissions as $name => $perm) {
        $perms[$module_info['springboard_target']['name']][$name] = strip_tags($perm['title']);
      }


    ksort($perms);

    $form['perm'] = array(
      '#type' => 'select',
      '#options' => $perms,
      '#title' => t('Permission'),
      '#default_value' => $this->options['perm'],
      '#description' => t('Only users with the selected permission flag will be able to access this display. Note that users with "access all views" can see any view, regardless of other permissions.'),
    );
  }
}