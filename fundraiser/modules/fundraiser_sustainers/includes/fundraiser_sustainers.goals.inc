<?php

/**
 * Implements hook_webform_goals_list_context().
 *
 * Provide custom tracking metrics and UI sugar for donation forms.
 */
function fundraiser_sustainers_webform_goals_list_metrics($selected_context, $form_state, $goal = array()) {

  // this metric should only be available to donation forms selected in a "single" context
  // or groups if the node type is also a donation node type.
  // or form groups if all forms selected in the group are fundraiser enabled.
  if (in_array($selected_context['id'], array('single', 'group', 'form_group'))) {
    $is_fundraiser_goal = fundraiser_webform_is_fundraiser_goal($selected_context, $form_state, $goal);
    if ($is_fundraiser_goal) {
      $metrics['fundraiser_sustainer_count'] = array(
        'id' => 'fundraiser_sustainer_count',
        'name' => t('Number of sustainer conversions'),
        'description' => t('Goal progress will be calculated based on the number of sustainer conversions.'),
        'form_text' => array(
          'target_value' => array(
            'title' => t('Total number of sustainer conversions required to meet this goal?'),
            'description' => t('Goal progress represents the total number of sustainer conversions as a percentage of this number.'),
          ),
          'seed' => array(
            'title' => t('Seed goal progress with this value'),
            'description' => t('Add this number to the current total sustainer conversions when calculating goal progress.'),
          ),
          'display_threshold' => array(
            'title' => t('Minimum total sustainer conversions required to show progress'),
            'description' => t('The goal progress bar will be hidden until the total sustainer conversions reach this number.') . ' ' .
              t('If a starter message exists, it will be shown in the template\'s place.'),
          ),
        ),
      );
      return $metrics;
    }
  }
}

/**
 * Implements hook_webform_goals_track_metric().
 */
function fundraiser_sustainers_webform_goals_track_metric($goal, $params = array()) {
  $data = array(
    'count' => '',
  );

  if ($goal['metric'] === 'fundraiser_sustainer_count') {
    if (!empty($goal['extra']['selected_form_id'])) {
      $data['count'] = _fundraiser_sustainers_get_total_sustainers_by_nid($goal['extra']['selected_form_id'], $goal['start_date'], $goal['end_date']);
    }
    // group goal handling. since we have multiple nodes we query the db for each.
    // Bet the database loves this. Can this be improved?
    else {
      if (!empty($goal['extra']['selected_node_type'])) {
        $nids = _webform_goals_nids_by_type($goal['extra']['selected_node_type']);
      }
      elseif (!empty($goal['extra']['selected_form_ids'])) {
        $nids = $goal['extra']['selected_form_ids'];
      }

      if (is_array($nids)) {
        foreach ($nids as $nid) {
          $sustainers = _fundraiser_sustainers_get_total_sustainers_by_nid($nid, $goal['start_date'], $goal['end_date']);

          if ($sustainers) {
            $data['count'] += $sustainers;
          }
        }
      }
    }
    if (empty($data['count'])) {
      $data['count'] = 0;
    }
    webform_goals_calculate_progress($data, $goal);
    return $data;
  }
}
