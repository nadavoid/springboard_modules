<?php

/**
 * @file
 * Plugin for Twitter.
 */

$plugin = array(
  'name' => 'twitter',
  'title' => t('Twitter'),
  'settings' => 'sb_social_twitter_settings',
  'admin_settings' => 'sb_social_twitter_admin_settings',
  'process' => 'sb_social_twitter_process_settings',
  'admin_setings_submit' => 'sb_social_twitter_admin_settings_submit',
  'share_config' => 'sb_social_twitter_share_js_config',
  'defaults' => 'sb_social_twitter_defaults',
  'uninstall' => 'sb_social_twitter_uninstall',
  'token_info' => sb_social_twitter_token_info(),
  'token_replace' => 'sb_social_twitter_token_replace',
  'button_contents' => sb_social_twitter_custom_button_contents(),
);

$use_addthis = variable_get('springboard_social_use_addthis', FALSE);
if ($use_addthis) {
  $plugin['js'] = 'plugins/social_networks/twitter/twitter.share.js';
}
else {
  $plugin['js'] = FALSE;
}


function sb_social_twitter_token_info() {
  $twitter_tokens['twitter_share_link'] = array(
    'name' => t('Twitter Share link'),
    'description' => t('Twitter tweet link'),
  );
  $twitter_tokens['twitter_message'] = array(
    'name' => t('Twitter tweet message'),
    'description' => t('Message content for Twitter tweet'),
  );
  return $twitter_tokens;
}

function sb_social_twitter_token_replace($tokens, &$replacements, $settings, $node = FALSE, $submission_id = FALSE, $extra = FALSE) {
  foreach ($tokens as $key => $token) {
    switch ($key) {
      case 'twitter_share_link':
        $vars['service'] = 'twitter';
        $vars['contents'] = sb_social_twitter_custom_button_contents();
        $use_addthis = variable_get('springboard_social_use_addthis', FALSE);
        if ($use_addthis) {
          $vars['extra'] = '';
          if ($extra && function_exists($extra)) {
            $vars['extra'] = $extra($settings['data']['twitter'], $node, $submission_id);
          }
        }

        $button = theme('sb_social_share_button', $vars);
        $replacements[$token] = theme('sb_social_share', array('buttons' => $button));
        break;

      case 'twitter_message':
        $replacements[$token] = !empty($settings['data']['twitter']['message']) ? _sb_social_replace_tokens($settings['data']['twitter']['message'], $node, $submission_id) : '';
        break;
    }
  }
}

function sb_social_twitter_defaults() {
  $settings = array(
    'message' => variable_get('springboard_social_twitter_message', '%title'),
  );
  return $settings;
}

function sb_social_twitter_settings(&$form, $enabled_services = array(), $settings = array(), $token_set = array('all')) {
  $form['twitter_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Twitter Settings'),
    '#access' => in_array('twitter', $enabled_services),
    '#collapsible' => TRUE,
  );
  // message
  $form['twitter_settings']['twitter_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Message'),
    '#description' => t('Custom twitter message text.'),
    '#default_value' => isset($settings['message']) ? $settings['message'] : '',
  );

  if (!module_exists('springboard_token_set')) {
    $form['twitter_settings']['tokens'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available Tokens'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['twitter_settings']['tokens']['token_help'] = array(
      '#type' => 'item',
      '#title' => t('Drupal tokens'),
      '#description' => theme('token_tree', array('token_types' => $token_set, 'recursion_limit' => 2, 'click_insert' => FALSE)),
    );
  }
}

function sb_social_twitter_admin_settings(&$form) {
  $form['services']['twitter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Twitter'),
    '#states' => array(
      // Hide the settings when facebook checkbox is not selected.
      'invisible' => array(
        ':input[name="springboard_social_services[twitter]"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['services']['twitter']['icon'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom icon settings'),
  );
  $form['services']['twitter']['icon']['springboard_social_twitter_custom_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom icon text'),
    '#default_value' => variable_get('springboard_social_twitter_custom_text', ''),
  );
  $form['services']['twitter']['icon']['springboard_social_twitter_custom_icon'] = array(
    '#type' => 'managed_file',
    '#title' => t('Upload custom icon'),
    '#description' => t('Custom share icon'),
    '#default_value' => variable_get('springboard_social_twitter_custom_icon', ''),
    '#upload_location' => 'public://social_images',
    '#upload_validators' => array(
      'file_validate_extensions' => array('gif png jpg jpeg'),
    ),
    '#theme' => 'sb_social_share_image_preview',
  );
  $form['services']['twitter']['springboard_social_twitter_message'] = array(
    "#type" => 'textfield',
    '#title' => t('Default twitter share message'),
    '#default_value' => variable_get('springboard_social_twitter_message', '%title'),
  );
}

function sb_social_twitter_admin_settings_submit($form, $form_state) {
  if ($form_state['values']['springboard_social_twitter_custom_icon']) {
    $value = $form_state['values']['springboard_social_twitter_custom_icon'];
    $file = file_load($value);
    if ($file && !$file->status) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      file_usage_add($file, 'sb_social', 'twitter_icon', 1);
    }
  }
}

function sb_social_twitter_process_settings(&$data, $form_state) {
  $data['twitter'] = array(
    'message' => !empty($form_state['values']['twitter_message']) ? $form_state['values']['twitter_message'] : '',
  );
}

function sb_social_twitter_share_js_config($settings, $node = FALSE, $sid = FALSE) {
  $settings = array(
    'twitter_message' => _sb_social_replace_tokens($settings['data']['twitter']['message'], $node, $sid),
  );
  drupal_add_js(array('sb_social' => $settings), 'setting');
}

function sb_social_twitter_uninstall() {
  variable_del('springboard_social_twitter_message');
}

function sb_social_twitter_custom_button_contents() {
  $contents = '';
  $icon = variable_get('springboard_social_twitter_custom_icon', '');
  $text = variable_get('springboard_social_twitter_custom_text', '');
  $use_addthis = variable_get('springboard_social_use_addthis', FALSE);

  if (!empty($icon)) {
    $icon_file = file_load($icon);
    if (isset($icon_file->uri)) {
      $url = file_create_url($icon_file->uri);
      $contents .= '<img src="' . $url . '">';
    }
  }
  elseif (empty($icon) && empty($text) && !$use_addthis) {
    $contents .= '<img src="' . base_path() . drupal_get_path('module', 'sb_social') . '/img/tr.png">';
  }
  if ($text) {
    $contents .= '<span>' . $text . '</span>';
  }
  return $contents;
}
