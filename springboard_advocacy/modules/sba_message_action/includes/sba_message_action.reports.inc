<?php

/**
 * Callback handler for 'node/%/performance' menu item.
 *
 * Generates and outputs deliverability reports for a particular message action.
 */
function sba_message_action_performance_page($nid) {
  // CSS and JS assets
  drupal_add_css(drupal_get_path('module', 'sba_message_action') . '/css/message-performance.css');
  drupal_add_js(drupal_get_path('module', 'sba_message_action') .
    '/js/message-performance.js');

  // Load up the message action node.
  $node = node_load($nid);

  // API Client instance
  $client = springboard_advocacy_api_call();

  // Try to make the API call and if it fails return a response.
  $responses = sba_message_action_report_cache($client, $node->advocacy_id);
  // Check the status of the API call return and if it's an error return that to the browser.
  if (isset($responses['report']->error)):
    return 'Sorry, no performance metrics are available for this message.';
  endif;

  $deliverablity_targets = sba_message_action_performance_targets_table($responses, $node);

  return $deliverablity_targets;
}

/**
 * Helper function to convert target deliverability report data into
 * a data structure that can be rendered in an HTML table.
 */
function sba_message_action_performance_targets_table($responses, $node) {

  // Set the pager to a sensible default if not set
  $pager_limit = isset($_GET['limit']) && is_numeric($_GET['limit']) ?
  $_GET['limit']:25;

  // The minimum page amount allowed
  $min_pager = 5;

  // If the page limit is less than the minimum, enforce the minimum
  if($pager_limit < $min_pager){
    $pager_limit = $min_pager;
  }
  // If the limit is set to all, set a number for it.
  elseif (isset($_GET['limit']) && $_GET['limit'] === 'all') {
    $pager_limit = 100000;
  }

  $current_url = current_path();
  $url_query_params = drupal_get_query_parameters(NULL, $exclude = array('q','limit'));
  $query_string_rebuild = '';

  foreach($url_query_params as $key => $param) {
    $query_string_rebuild.=$key."=".$param."&";
  }
  $form_url_with_input = $current_url.'?'.$query_string_rebuild."limit=";

  $targets = !empty($responses['report']->data->targets) ? $responses['report']->data->targets : array();
  $targets = sba_message_action_pager_default_initialize($targets, $pager_limit);

  $rows = array();
  $advocacy_id = $node->advocacy_id;
  foreach ($targets as $target) {
    // Get the target deliverability metrics for this target
    $rows[] = array(
      $target->salutation . ' ' . $target->first_name . ' ' . $target->last_name,
      $target->party,
      $target->state,
      $target->district_code,
      $target->deliverability->delivered,
      $target->deliverability->failed,
      $target->deliverability->hold,
      $target->deliverability->totalMessages,
    );
  }

  // Define the table header
  $header = array('Name', 'Party', 'State', 'District', 'Delivered Messages', 'Failed Messages', 'On Hold Messages','Total Messages Processed');
  $tabs = theme('messsage_deliverability_tabs', array(
    'nodeId' => $node->nid
  ));

  // Get the advocacy endpoint for use in the legislative results download link
  $springboard_advocacy_endpoint = variable_get
  ('springboard_advocacy_server_url');

  // Get the advocacy access token for use in the legislative results
  // download link
  $auth_token = variable_get('springboard_advocacy_access_token')->access_token;

  $output = theme('messsage_deliverability_dashboard', array(
    'targets_table_header'           => $header,
    'targets'                        => $rows,
    'report'                         => $responses['report'],
    'nodeId'                         => $node->nid,
    'tabs'                           => $tabs,
    'deliverability_cache_timestamp' => $responses['deliverability_cache_timestamp'],
    'legislative_results_download_link' =>
      preg_replace("@[^://]//@", "/", "$springboard_advocacy_endpoint/api/v1/deliverability/action/$advocacy_id/download?access_token=$auth_token"),
    'form_url_with_input'            => $form_url_with_input,
  ));

  return $output;
}

/**
 * Modified version of pager.inc function pager_default_initialize().
 *
 * Slices the array of targets into pageable segments.
 *
 * @param $rows
 * @param int $limit
 * @param int $element
 * @return array
 */
function sba_message_action_pager_default_initialize($rows, $limit = 9, $element = 0) {
  global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;
  $page = pager_find_page($element);
  // We calculate the total of pages as ceil(items / limit).
  $pager_total_items[$element] = count($rows);
  $pager_total[$element] = ceil($pager_total_items[$element] / $limit);
  $pager_page_array[$element] = max(0, min($page, ((int)$pager_total[$element]) - 1));
  $pager_limits[$element] = $limit;

  return array_slice($rows, $pager_page_array[$element] * $limit, $limit, TRUE);
}

/**
 * Implements hook_preprocess()
 * @param $variables
 * @throws \Exception
 */
function sba_message_action_preprocess_messsage_deliverability_dashboard(&$vars) {
  // Sanitize the targets data
  foreach ($vars['targets'] as $key => $data) {
    foreach ($data as $item_key => $value) {
      $vars['targets'][$key][$item_key] = check_plain($value);
    }
  }

  // Sanitize report data
  $vars['total_messages_processed'] = isset($vars['report']->data->totalMessagesProcessed) ? check_plain($vars['report']->data->totalMessagesProcessed) : 0;
  $vars['total_messages_generated'] = isset($vars['report']->data->totalMessagesGenerated) ? check_plain($vars['report']->data->totalMessagesGenerated) : 0;
  $vars['delivered'] = isset($vars['report']->data->delivered) ? check_plain($vars['report']->data->delivered) : 0;

  $vars['failed'] = isset($vars['report']->data->failed) ? check_plain($vars['report']->data->failed) : 0;
  $vars['hold'] = isset($vars['report']->data->hold) ? check_plain($vars['report']->data->hold) : 0;
  $vars['undeliverable'] = isset($vars['report']->data->undeliverable) ? check_plain($vars['report']->data->undeliverable) : 0;

  $vars['total_messages_processed'] = number_format($vars['total_messages_processed']);
  $vars['total_messages_generated'] = number_format($vars['total_messages_generated']);
  $vars['delivered'] = number_format($vars['delivered']);
  $vars['failed'] = number_format($vars['failed']);
  $vars['hold'] = number_format($vars['hold']);
  $vars['undeliverable'] = number_format($vars['undeliverable']);

  $vars['percentage'] = isset($vars['report']->data->deliverabilityIndexPercentage) ? check_plain($vars['report']->data->deliverabilityIndexPercentage) : 0;
  $vars['deliverability_cache_timestamp'] = isset($vars['deliverability_cache_timestamp']) ? check_plain($vars['deliverability_cache_timestamp']) : date('U');

  // Render targets table
  $targets_table = theme('table', array('header' => $vars['targets_table_header'], 'rows' => $vars['targets']));
  $targets_table .= theme('pager', array('quantity' => 1));

  $vars['targets_table'] = $targets_table;
}

function sba_message_action_report_cache($client, $advocacy_id) {
  $responses = &drupal_static(__FUNCTION__);
  if (empty($responses)) {
    if ($cache = cache_get($advocacy_id, 'cache_deliverability_report')) {
      $responses = $cache->data;
      $responses['deliverability_cache_timestamp'] = $cache->created;
      if (REQUEST_TIME > $cache->expire) {
        cache_clear_all($advocacy_id, 'cache_deliverability_report');
      }
    }
    else {
      $report = $client->invokeClientMethod('getTargetDeliverability', $advocacy_id);
      $responses['report'] = $report;
      $responses['deliverability_cache_timestamp'] = REQUEST_TIME;

      cache_set($advocacy_id, $responses, 'cache_deliverability_report', REQUEST_TIME + 600);

    }
  }

  return $responses;
}

function sba_message_action_target_cache($client, $target_id, $advocacy_id) {
  $responses = &drupal_static(__FUNCTION__);

  if (empty($responses)) {
    if ($cache = cache_get($advocacy_id . ':' . $target_id, 'cache_deliverability_target')) {
      $responses = $cache->data;
      if (REQUEST_TIME > $cache->expire) {
        cache_clear_all($advocacy_id . ':' . $target_id, 'cache_deliverability_target');
      }
    }
    else {
      $responses = $client->invokeClientMethod('getSingleTargetDeliverability', $advocacy_id, $target_id);
      cache_set($advocacy_id . ':' . $target_id, $responses, 'cache_deliverability_target', REQUEST_TIME + 600);
    }
  }

  return $responses;
}

/**
 * Helper function to set selected state on option if it matches the value
 * specified in the method input.
 */
function sba_message_action_pager_option_selected($input) {
  if(isset($_GET['limit']) && isset($input) && (is_numeric($input) || $input ==
  'all')) {
    if ($input == $_GET['limit']) {
      echo "selected";
    }
  }
}