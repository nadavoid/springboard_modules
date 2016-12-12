<?php

/**
 * @file
 * Plugin to display a personal campaign's donor roll.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Personal campaign donor roll'),
  'description' => t('Displays the P2P personal campaign donor roll.'),
  'category' => t('Springboard P2P'),
  // Despite having no "settings" we need this function to pass back a form,
  // or we'll loose the context and title settings.
  'edit form' => 'springboard_p2p_personal_campaign_roll_edit_form',
  'render callback' => 'springboard_p2p_personal_campaign_roll_render',
  'required context' => new ctools_context_required(t('Node'), 'node'),
);

/**
 * Render a single content pane.
 */
function springboard_p2p_personal_campaign_roll_render($subtype, $conf, $panel_args, $context) {
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
    if (!springboard_p2p_campaign_should_show_donor_roll($campaign)) {
      return;
    }
  }

  $comments = springboard_p2p_get_personal_campaign_comments($personal_campaign->nid, FALSE);
  if (count($comments) < 1) {
    return;
  }

  $goal_type = springboard_p2p_get_goal_type_from_campaign($campaign);

  if ($goal_type == 'amount') {
    $title = 'Recent donors';
    $header = '<thead><tr><td>Name</td><td>Amount</td></tr></thead>';
  }
  else {
    $title = 'Recent supporters';
    $header = '<thead><tr><td>Name</td></tr></thead>';
  }

  $output = array();

  $output[] = '<i class="fa fa-user donor-icon"></i>';

  $output[] = '<table class="personal-campaign-roll">';
  $output[] = $header;
  // Start the count.
  $zebra = 0;

  foreach ($comments as $comment) {

    // Define ospan/ even for use in the definition list.
    $zebra_class = ($zebra % 2) ? 'odd' : 'even';

    $output[] = '<tr class="personal-campaign-roll-donor ' . $zebra_class . '">';

    if ($comment['show_name'] && !empty($comment['donor_name'])) {
      $output[] = '<td>';
      $output[] = $comment['donor_name'];
      $output[] = '</td>';
    }
    else {
      $output[] = '<td>';
      $output[] = t('Anonymous');
      $output[] = '</td>';
    }

    if ($goal_type == 'amount') {
      $output[] = '<td>';

      if ($comment['action_type'] == 'fundraiser' && !empty($comment['amount'])) {
        $output[] = springboard_p2p_format_progress($comment['amount'] / 100, 'amount');
      }

      $output[] = '</td>';
    }

    $output[] = '</tr>';
    $zebra++;
  }

  $output[] = '</table>';

  // Build the content type block.
  $block = new stdClass();
  $block->title = $title;
  $block->content = implode('', $output);

  return $block;
}

/**
 * Settings form.
 */
function springboard_p2p_personal_campaign_roll_edit_form($form, &$form_state) {
  // No settings beyond context, which has already been handled.
  return $form;
}
