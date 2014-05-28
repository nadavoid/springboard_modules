<?php

/**
 * @file
 * Plugin to display a campaign header image.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Facebook login'),
  'description' => t('Displays the P2P Facebook login/register button.'),
  'category' => t('Springboard P2P'),
  // Despite having no "settings" we need this function to pass back a form,
  // or we'll loose the context and title settings.
  'edit form' => 'springboard_p2p_fboauth_login_edit_form',
  'render callback' => 'springboard_p2p_fboauth_login_render',
  // 'required context' => new ctools_context_required(t('Node'), 'node'),
);

/**
 * Render a single content pane.
 */
function springboard_p2p_fboauth_login_render($subtype, $conf, $panel_args, $context) {
  if (module_exists('fboauth')) {
    $campaign_id = springboard_p2p_get_campaign_id_from_request();

    // Build the content type block.
    $block = new stdClass();
    $block->title = 'Social login';
    $block->content = fboauth_action_display('connect', 'springboard_p2p/fb?p2p_cid=' . $campaign_id);

    return $block;
  }

}

/**
 * Settings form.
 */
function springboard_p2p_fboauth_login_edit_form($form, &$form_state) {
  // No settings beyond context, which has already been handled.
  return $form;
}