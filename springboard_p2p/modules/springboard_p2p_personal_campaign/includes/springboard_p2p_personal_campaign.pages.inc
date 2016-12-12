<?php

/**
 * @file
 * Page callbacks and associated helper functions.
 */

/**
 * Page callback for personal campaign donor dashboard.
 *
 * @param object $account
 *   User object loaded from uid in path.
 *
 * @param object $node
 *   donation form node.
 *
 * @param object $personal_campaign
 *   Personal campaign node.
 *
 * @return array
 *   Render array.
 */
function springboard_p2p_personal_campaign_donors_dashboard($account, $node, $personal_campaign) {
  drupal_set_title($node->title);
  $dashboard_contents = views_embed_view('personal_campaign_donor_dashboard', 'default', $node->nid, $personal_campaign->nid);
  return array(
   'content' => array(
     '#markup' => $dashboard_contents,
   ),
 );
}

/**
 * hook theme implementation for form table on donor edit page.
 */
function theme_springboard_p2p_personal_campaign_textfield_table($vars) {
  $element = $vars['element'];
  $rows = array();
  foreach (element_children($element) as $key) {
    $rows[] = array(
      array('data' => $element[$key]['edit_name']),
      array('data' => $element[$key]['donor_name']),
      array('data' => $element[$key]['amount']),
    );
  }

  $header = array(t('Edit Name'), t('Donor Name'), t('Amount'));
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Form builder for /campaign-donors menu callback.
 */
function springboard_p2p_personal_campaign_donors_edit($form_state, $edit, $node) {
  $donors= springboard_p2p_get_personal_campaign_donors($node->nid);
  // Only display the edit form if the campaign has donors.
  if (!empty($donors)) {
    $form['donors'] = array(
      '#prefix' => '<div id="donors">',
      '#suffix' => '</div>',
      '#tree' => TRUE,
      '#theme' => 'springboard_p2p_personal_campaign_textfield_table'
    );
    foreach ($donors as $donor) {
      $sid = $donor['sid'];
      $form['donors'][$sid] = array();
      $form['donors'][$sid]['donor_name'] = array(
        '#type' => 'textfield',
        '#default_value' => decode_entities($donor['donor_name']),
        '#disabled' => TRUE,
        '#states' => array(
          'enabled' => array(
            ":input[name='donors[$sid][edit_name]']" => array('checked' => TRUE),
          ),
        ),
      );
      $form['donors'][$sid]['edit_name'] = array(
        '#type' => 'checkbox',
      );
      $form['donors'][$sid]['amount'] = array(
        '#markup' =>  springboard_p2p_format_progress($donor['amount'] / 100, 'amount'),
      );
      $form['donors'][$sid]['keys'] = array(
        '#type' => 'value',
        '#value' => array(
          'sid' => $sid,
          'uid' => $donor['uid'],
        ),
      );
    }
    $form['personal_campaign_nid'] = array(
      '#type' => 'value',
      '#value' => $node->nid,
    );
    $form['submit'] = array
    (
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
    return $form;
  }
}


/**
 * Submit handler for the springboard_p2p_personal_campaign_donors_edit form.
 */
function springboard_p2p_personal_campaign_donors_edit_submit($form, &$form_state) {
  $donors = $form_state['input']['donors'];
  $personal_campaign_nid = $form_state['values']['personal_campaign_nid'];
  $updates = array();
  $names_changed = array();
  foreach ($donors as $sid => $donor) {
    if ($donor['edit_name']) {
      $names_changed[$sid] = array(
        '%from' => $form_state['values']['donors'][$sid]['donor_name'],
        '%to' => $donor['donor_name'],
      );
      $updates[] = array_merge($form_state['values']['donors'][$sid], $donor);
    }
  }
  if (!empty($updates)) {
   springboard_p2p_update_personal_campaign_donors($updates, $personal_campaign_nid);
   foreach ($names_changed as $name) {
    drupal_set_message(t("Updated %from to %to", $name)); 
   }
  }
}

/**
 * Update the donor_name field for a list of p2p personal campaign donors.
 * @param array $donors
 *  the list of donor names to update
 * @param int $nid
 *  the nid of the p2p personal campaign associated with the donors
 */
function springboard_p2p_update_personal_campaign_donors($donors, $nid) {
  foreach ($donors as $donor) {
    $result = db_update('springboard_p2p_personal_campaign_action')
      ->fields(array(
        'donor_name' => check_plain($donor['donor_name'], array()),
      ))
      ->condition('uid', $donor['keys']['uid'])
      ->condition('sid', $donor['keys']['sid'])
      ->condition('personal_campaign_nid', $nid)
      ->execute();
  }
}

/**
 * Get donor names for a given p2p personal campaign.
 * @param int $nid
 *  nid of the p2p personal campaign loaded from the menu router path
 * @return array
 *  list of all donors for the p2p personal campaign with the given nid
 */
function springboard_p2p_get_personal_campaign_donors($nid) {
  $query = db_select('springboard_p2p_personal_campaign_action', 'a')
    ->fields('a', array('uid', 'amount', 'donor_name', 'sid'))
    ->condition('status', 1)
    ->condition('show_name', 1)
    ->condition('personal_campaign_nid', $nid)
    ->orderBy('created', 'DESC');
  return $query->execute()->fetchAll(PDO::FETCH_ASSOC);
}