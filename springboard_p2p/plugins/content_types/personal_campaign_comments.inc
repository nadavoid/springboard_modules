<?php

/**
 * @file
 * Plugin to display a personal campaign's comments.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Personal campaign comments'),
  'description' => t('Displays the P2P personal campaign comments.'),
  'category' => t('Springboard P2P'),
  // Despite having no "settings" we need this function to pass back a form,
  // or we'll loose the context and title settings.
  'edit form' => 'springboard_p2p_personal_campaign_comments_edit_form',
  'render callback' => 'springboard_p2p_personal_campaign_comments_render',
  'required context' => new ctools_context_required(t('Node'), 'node'),
);

/**
 * Render a single content pane.
 */
function springboard_p2p_personal_campaign_comments_render($subtype, $conf, $panel_args, $context) {
  $personal_campaign = NULL;
  if (empty($context) || empty($context->data)) {
    if (isset($_GET['p2p_pcid']) && is_numeric($_GET['p2p_pcid'])) {
      $personal_campaign = node_load($_GET['p2p_pcid']);
    }
    else {
      return;
    }
  }
  else {
    $personal_campaign = $context->data;
  }

  if (!isset($personal_campaign->type) || $personal_campaign->type != 'p2p_personal_campaign' || !isset($personal_campaign->field_p2p_campaign[$personal_campaign->language][0]['target_id'])) {
    return;
  }
  else {
    $campaign = node_load($personal_campaign->field_p2p_campaign[$personal_campaign->language][0]['target_id']);
    if (!springboard_p2p_campaign_should_show_comments($campaign)) {
      return;
    }
  }

  $comments = springboard_p2p_get_personal_campaign_comments($personal_campaign->nid, TRUE);
  $output = array();

  // Start the count.
  $zebra = 0;

  $output[] = '<ul class="comment-wrapper">';

  foreach ($comments as $comment) {

    // Define odd / even for use in the definition list.
    $zebra_class = ($zebra % 2) ? 'odd' : 'even';

    $output[] = '<li class="personal-campaign-comment-list ' . $zebra_class . '">';

    $output[] = '<span class="p2p-comment-name-amount-wrapper">';

    if ($comment['show_name'] && !empty($comment['donor_name'])) {
      $output[] = '<h3 class="p2p-comment-name">';
      $output[] = $comment['donor_name'];
      $output[] = '</h3>';
    }
    else {
      $output[] = '<h3 class="p2p-comment-name">';
      $output[] = t('Anonymous');
      $output[] = '</h3>';
    }

    if ($comment['action_type'] == 'fundraiser' && !empty($comment['amount'])) {
      $output[] = '<span class="p2p-comment-amount">';
      $output[] = springboard_p2p_format_progress($comment['amount'] / 100, 'amount');
      $output[] = '</span>';
    }

    $output[] = '<span class="p2p-comment-date">';
    // Custom Date format.
    $output[] = format_date($comment['created'], "custom", "m / j / Y");
    $output[] = '</span>';

    // p2p-comment-name-amount
    $output[] = '</span>';

    $output[] = '<span class="p2p-comment-text-wrapper">';

    if (!empty($comment['comment'])) {
      $output[] = '<span class="comment-arrow"></span>';
      $output[] = '<p class="p2p-comment-text">';
      $output[] = check_plain($comment['comment']);
      $output[] = '</p>';
    }

    // p2p-comment-text-wrapper
    $output[] = '</span>';

    $output[] = '</li>';
    $zebra++;
  }

  $output[] = '</ul>';

  // Build the content type block.
  $block = new stdClass();
  $block->title = 'Comments From Donors';
  $block->content = implode('', $output);
  return $block;

}

/**
 * Settings form.
 */
function springboard_p2p_personal_campaign_comments_edit_form($form, &$form_state) {
  // No settings beyond context, which has already been handled.
  return $form;
}
