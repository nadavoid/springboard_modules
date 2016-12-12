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

    $url = 'p2p/fb?p2p_cid=' . $campaign_id;
    if (isset($_GET['campaign_name'])) {
      $url .= '&campaign_name=' . filter_xss($_GET['campaign_name']);
    }
    if (isset($_GET['campaign_url'])) {
      $url .= '&campaign_url=' . filter_xss($_GET['campaign_url']);
    }
    if (isset($_GET['campaign_goal'])) {
      $url .= '&campaign_goal=' . filter_xss($_GET['campaign_goal']);
    }

    $output = array();

    // Wrap the button using an output array.
    $output[] = '<div class="fb-oauth-wrapper">';
    $output[] = fboauth_action_display('connect', $url);
    $output[] = '</div>';

    // Build the content type block.
    $block = new stdClass();
    $block->title = 'Social login';
    $block->content = implode('', $output);

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
