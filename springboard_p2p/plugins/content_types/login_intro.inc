<?php
/**
 * @file
 * Plugin to display a get started pane.
 */

$plugin = array(
  'single' => TRUE,
  'title' => t('Custom Login Pane'),
  'description' => t('Login Intro'),
  'category' => t('Springboard P2P'),
  // A category to put this under.
  'edit form' => 'springboard_p2p_login_intro_edit_form',
  // A function that will return the settings form for the pane.
  'render callback' => 'springboard_p2p_login_intro_render',
  // A function that will return the renderable content.
  'admin info' => 'springboard_p2p_login_intro_admin_info',
  // A function that will return the information displayed on the admin screen (optional).
  'defaults' => array( // Array of defaults for the settings form.
    'text' => '',
  ),
  'all contexts' => TRUE,
  // This is NEEDED to be able to use substitution strings in your pane.
);

/**
 * An edit form for the pane's settings.
 */
function springboard_p2p_login_intro_edit_form($form, &$form_state) {
  // No settings beyond context, which has already been handled.
  return $form;
}

/**
 * Run-time rendering of the body of the block (content type).
 *
 * See ctools_plugin_examples for more advanced info.
 */
function springboard_p2p_login_intro_render($subtype, $conf, $args, $contexts) {

  // Title.
  $title = t('Already a Supporter?');

  // Pull the login message text from P2P Settings.
  $logintext = variable_get('springboard_p2p_login_message_area');

  // Define the reg link passing the string id.
  $login_url = 'p2p/login';

  $parameters = drupal_get_query_parameters();
  $query = array();
  $output = array();

  // Query the url string campaign id if it's not empty. Confirm
  // input is numeric to prevent XSS.
  if (!empty($parameters['p2p_cid']) && is_numeric($parameters['p2p_cid'])) {
    $query['p2p_cid'] = filter_xss($parameters['p2p_cid']);
  }
  if (isset($parameters['campaign_name'])) {
    $query['campaign_name'] = filter_xss($parameters['campaign_name']);
  }
  if (isset($parameters['campaign_url'])) {
    $query['campaign_url'] = filter_xss($parameters['campaign_url']);
  }
  if (isset($parameters['campaign_goal'])) {
    $query['campaign_goal'] = filter_xss($parameters['campaign_goal']);
  }

  $icon = array(
    '#type' => 'html_tag',
    '#tag' => 'i',
    '#attributes' => array(
      'class' => array('fa', 'fa-cog', 'get-started-spinner'),
    ),
    '#value' => '',
  );
  $link = array(
    '#theme' => 'link',
    '#path' => $login_url,
    '#text' => t('Login') . render($icon),
    '#options' => array(
      'attributes' => array(
        'class' => array('get-started-link'),
      ),
      'html' => TRUE,
      'query' => $query,
    ),
  );
  $output = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('login-text-wrapper'),
    ),
    'link' => $link,
  );

  // Initial content is blank.
  // Build the content type block.
  $block = new stdClass();
  $block->title = $title;
  $block->content = render($output);
  return $block;

}
