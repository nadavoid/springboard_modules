<?php

/**
 * @file
 * Plugin for Facebook/OpenGraph.
 */

$plugin = array(
  'name' => 'facebook',
  'title' => t('Facebook'),
  'settings' => 'sb_social_facebook_settings',
  'admin_settings' => 'sb_social_facebook_admin_settings',
  'admin_setings_submit' => 'sb_social_facebook_admin_settings_submit',
  'process' => 'sb_social_facebook_process_settings',
  'share_config' => 'sb_social_facebook_share_opengraph_config',
  'js' => FALSE,
  'defaults' => 'sb_social_facebook_defaults',
  'uninstall' => 'sb_social_facebook_uninstall',
  'token_info' => sb_social_facebook_token_info(),
  'token_replace' => 'sb_social_facebook_token_replace',
  'metatag_view_alter' => 'facebook_metatag_view_alter',
  'button_contents' => sb_social_facebook_custom_button_contents(),
);

/**
 * Alter Metatag Opengraph tags.
 *
 * @param $output
 *   Metatag view output.
 *
 * @param $settings
 *   Share content configuration settings.
 */
function facebook_metatag_view_alter($settings, &$output) {
  if (module_exists('metatag_opengraph')) {
    $meta = array(
      'og:title' => !empty($settings['title']) ? $settings['title'] : '',
      'og:description' => !empty($settings['description']) ? $settings['description'] : '',
      'og:image' => !empty($settings['image']) ? $settings['image'] : '',
    );
    // If we have values remove Metatag Opengraph's corresponding tag.
    // This prevents duplicate tags.
    foreach ($meta as $key => $value) {
      if ($value && isset($output[$key])) {
        unset($output[$key]);
      }
    }
  }
}

function sb_social_facebook_token_info() {
  $facebook_tokens = array();
  $facebook_tokens['facebook_share_link'] = array(
    'name' => t('Facebook Share link'),
    'description' => t('Facebook Share link'),
  );
  // fb title
  $facebook_tokens['facebook_title'] = array(
    'name' => t('Facebook share title'),
    'description' => t('Title content for Facebook sharing'),
  );
  // fb description
  $facebook_tokens['facebook_description'] = array(
    'name' => t('Facebook share description'),
    'description' => t('Description content for Facebook sharing'),
  );
  // fb image
  $facebook_tokens['facebook_image'] = array(
    'name' => t('Facebook share image'),
    'description' => t('Image content for Facebook sharing'),
  );

  return $facebook_tokens;
}

function sb_social_facebook_token_replace($tokens, &$replacements, $settings, $node = FALSE, $submission_id = FALSE, $extra = FALSE) {
  foreach ($tokens as $key => $token) {
    switch ($key) {
      case 'facebook_share_link':
        $vars['service'] = 'facebook';
        $vars['contents'] = sb_social_facebook_custom_button_contents();

        $use_addthis = variable_get('springboard_social_use_addthis', FALSE);
        if ($use_addthis) {
          $vars['extra'] = '';
          if ($extra && function_exists($extra)) {
            $vars['extra'] = $extra($settings['data']['facebook']);
          }
        }
        $button = theme('sb_social_share_button', $vars);
        $replacements[$token] = theme('sb_social_share', array('buttons' => $button));
        break;

      case 'facebook_title':
        $replacements[$token] = !empty($settings['data']['facebook']['title']) ? _sb_social_replace_tokens($settings['data']['facebook']['title'], $node, $submission_id) : '';
        break;

      case 'facebook_description':
        $replacements[$token] = !empty($settings['data']['facebook']['description']) ? _sb_social_replace_tokens($settings['data']['facebook']['description'], $node, $submission_id) : '';
        break;

      case 'facebook_image':
        $fid = $settings['data']['facebook']['image'];
        $image_url = FALSE;
        if ($fid) {
          $file = file_load($fid);
          if (isset($file->uri)) {
            $image_url = file_create_url($file->uri);
          }
        }
        $replacements[$token] =  $image_url ? $image_url : '';
        break;
    }
  }

}

function sb_social_facebook_defaults() {
  $settings = array(
    'title' => variable_get('springboard_social_facebook_title', '%title'),
    'description' => variable_get('springboard_social_facebook_description', '%title'),
    'image' => variable_get('springboard_social_facebook_image', ''),
  );
  return $settings;
}

function sb_social_facebook_admin_settings(&$form) {
  $image = variable_get('springboard_social_facebook_image', FALSE);
  $form['services']['facebook'] = array(
    '#type' => 'fieldset',
    '#title' => t('Facebook Settings'),
    '#states' => array(
      // Hide the settings when facebook checkbox is not selected.
      'invisible' => array(
        ':input[name="springboard_social_services[facebook]"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['services']['facebook']['icon'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom icon settings'),
  );
  $form['services']['facebook']['icon']['springboard_social_facebook_custom_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom icon text'),
    '#default_value' => variable_get('springboard_social_facebook_custom_text', ''),
  );
  $form['services']['facebook']['icon']['springboard_social_facebook_custom_icon'] = array(
    '#type' => 'managed_file',
    '#title' => t('Upload custom icon'),
    '#description' => t('Custom share icon'),
    '#default_value' => variable_get('springboard_social_facebook_custom_icon', ''),
    '#upload_location' => 'public://social_images',
    '#upload_validators' => array(
      'file_validate_extensions' => array('gif png jpg jpeg'),
    ),
    '#theme' => 'sb_social_share_image_preview',
  );
  $form['services']['facebook']['springboard_social_facebook_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Facebook share title'),
    '#default_value' => variable_get('springboard_social_facebook_title', '%title'),
  );
  $form['services']['facebook']['springboard_social_facebook_description'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Facebook share description'),
    '#default_value' => variable_get('springboard_social_facebook_description', '%title'),
  );
  $form['services']['facebook']['springboard_social_facebook_image'] = array(
    '#type' => 'managed_file',
    '#title' => 'Image upload',
    '#description' => t('Upload an image to display in share content'),
    '#default_value' => $image ? $image : variable_get('logo', ''),
    '#upload_location' => 'public://social_images',
    '#upload_validators' => array(
      'file_validate_extensions' => array('gif png jpg jpeg'),
    ),
    '#theme' => 'sb_social_share_image_preview',
  );
}

function sb_social_facebook_admin_settings_submit($form, $form_state) {
  if ($form_state['values']['springboard_social_facebook_image']) {
    $value = $form_state['values']['springboard_social_facebook_image'];
    $file = file_load($value);
    if ($file && !$file->status) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      file_usage_add($file, 'sb_social', 'admin_default', 1);
    }
  }
  if ($form_state['values']['springboard_social_facebook_custom_icon']) {
    $value = $form_state['values']['springboard_social_facebook_custom_icon'];
    $file = file_load($value);
    if ($file && !$file->status) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      file_usage_add($file, 'sb_social', 'facebook_icon', 1);
    }
  }
}

function sb_social_facebook_settings(&$form, $enabled_services = array(), $settings = array(), $token_set = array('all')) {
  $image = $settings['image'];
  $form['fb_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Facebook Settings'),
    '#access' => in_array('facebook', $enabled_services),
    '#collapsible' => TRUE,
  );
  // title
  $form['fb_settings']['fb_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('Facebook Share title'),
    '#default_value' => isset($settings['title']) ? $settings['title'] : '',
  );
  // description
  $form['fb_settings']['fb_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Facebook Share description'),
    '#default_value' => isset($settings['description']) ? $settings['description'] : '',
  );
  // image
  $form['fb_settings']['fb_image'] = array(
    '#type' => 'managed_file',
    '#title' => 'Image upload',
    '#description' => t('Upload an image to display in share content'),
    '#default_value' => $image ? $image : variable_get('logo', ''),
    '#upload_location' => 'public://social_images',
    '#upload_validators' => array(
      'file_validate_extensions' => array('gif png jpg jpeg'),
    ),
    '#theme' => 'sb_social_share_image_preview',
  );

  if (!module_exists('springboard_token_set')) {
    $form['fb_settings']['tokens'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available Tokens'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['fb_settings']['tokens']['token_help'] = array(
      '#type' => 'item',
      '#title' => t('Drupal tokens'),
      '#description' => theme('token_tree', array(
        'token_types' => $token_set,
        'recursion_limit' => 2,
        'click_insert' => FALSE,
      )),
    );
  }
}

function sb_social_facebook_process_settings(&$data, $form_state) {
  $data['facebook'] = array(
    'title' => !empty($form_state['values']['fb_title']) ? $form_state['values']['fb_title'] : '',
    'description' => !empty($form_state['values']['fb_description']) ? $form_state['values']['fb_description'] : '',
    'image' => '',
  );

  if (!empty($form_state['values']['fb_image'])) {
    $id = is_numeric($form_state['values']['id']) ? $form_state['values']['id'] : time();
    $value = $form_state['values']['fb_image'];
    $file = file_load($value);
    if ($file && !$file->status) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      file_usage_add($file, 'sb_social', 'setting', $form_state['values']['id']);
    }

    $file = file_load($form_state['values']['fb_image']);
    $data['facebook']['image'] = $file->fid;
  }
}

/**
 * Configure share buttons for Facebook.
 *
 * Add OpenGraph meta tags used by Facebook to define custom title, description,
 * and image content on shares.
 *
 * Facebook and a few other services use opengraph tags to define custom share
 * content instead of javascript configuration objects.
 *
 */
function sb_social_facebook_share_opengraph_config($settings, $node = FALSE, $sid = FALSE) {
  $facebook = $settings['data']['facebook'];
  $meta = array(
    'title' => !empty($facebook['title']) ? $facebook['title'] : '',
    'description' => !empty($facebook['description']) ? $facebook['description'] : '',
  );
  // Facebook gets angry if you output an empty image opengraph tag.
  if (!empty($facebook['image'])) {
    $file = file_load($facebook['image']);
    if (isset($file->uri) && file_valid_uri($file->uri)) {
      $meta['image'] = file_create_url($file->uri);
    }
  }

  foreach ($meta as $field => $value) {
    $meta[$field] = _sb_social_replace_tokens($value, $node, $sid);
  }
  foreach ($meta as $id => $content) {
    $data = array(
      '#type' => 'html_tag',
      '#tag' => 'meta',
      '#attributes' => array(
        'property' => "og:$id",
        'content' => $content,
      ),
    );
    drupal_add_html_head($data, 'opengraph_' . $id);
  }
}

function sb_social_facebook_uninstall() {
  // Remove variables.
  variable_del('springboard_social_facebook_title');
  variable_del('springboard_social_facebook_description');
  variable_del('springboard_social_facebook_image');

}

function sb_social_facebook_custom_button_contents() {
  $contents = '';
  $icon = variable_get('springboard_social_facebook_custom_icon', '');
  $text = variable_get('springboard_social_facebook_custom_text', '');
  $use_addthis = variable_get('springboard_social_use_addthis', FALSE);

  if (!empty($icon)) {
    $icon_file = file_load($icon);
    if (isset($icon_file->uri)) {
      $url = file_create_url($icon_file->uri);
      $contents .= '<img src="' . $url . '">';
    }
  }
  elseif (empty($icon) && empty($text) && !$use_addthis) {
    $contents .= '<img src="' . base_path() . drupal_get_path('module', 'sb_social') . '/img/bk.png">';
  }
  if ($text) {
    $contents .= '<span>' . $text . '</span>';
  }
  return $contents;
}
