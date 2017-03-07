<?php
/**
 * @file springbord_advocacy_views_plugin_query.inc
 *
 */

/**
 * Class springboard_advocacy_views_plugin_query
 * Override views default db query with advocacy REST API query.
 * Queries transaction server for Advocacy targets and custom groups
 *
 */
class springboard_advocacy_views_plugin_query extends views_plugin_query {

  /**
   * AdvocacyApiCall object.
   *
   * @var AdvocacyApiCall $apiCall
   */
  public $apiCall;

  /**
   * @param $base_table
   * @param $base_field
   * @param $options
   */
  function init($base_table, $base_field, $options) {
    parent::init($base_table, $base_field, $options);
    $this->apiCall = springboard_advocacy_api_call();
  }

  /**
   * @param view $view
   */
  function build(&$view) {
    $view->init_pager($view);
    $this->pager->query();
    $view->build_info['springboard_advocacy_request'] = $this->apiCall;
    $view->build_info['target_ids'] = array();
    // Helpful for the "add all link".
    if (isset($view->query->search_terms)) {
      $view->build_info['springboard_advocacy_search'] = $view->query->search_terms;
    }
  }

  function execute(&$view) {
    $api_call = $view->build_info['springboard_advocacy_request'];

    $start = microtime(TRUE);

    // Should we pull the terms from $view->query? Why use $view->build_info?
    $fields = !empty($view->query->search_terms) ? $view->query->search_terms : NULL;

    // If a non-exposed class_name search is present and it is the only search
    // suppress results because no search has actually been performed yet
    // Will need to further limit this by view block name so the main admin
    // screens have results by default.
    if ((count($fields) == 1) && isset($fields['class_name'])) {
      //$suppress_results = $view->filter['search_class_name']->options['exposed'] == FALSE;
    }

    if (!empty($fields) && empty($suppress_results)) {
      foreach ($fields as $field_name => $search_terms) {

        // Is it a multiple value checkbox or select?
        // Then format in a way the api expects.
        if (is_array($search_terms)) {
          $fields[$field_name] = implode('|', $search_terms);
        }
        else {

          // Strip leading and multiple whitespaces.
          $search_terms = trim($search_terms);
          $search_terms = preg_replace('/\s+/', ' ', $search_terms);

          // Parsing for the committee:"committee name" search option.
          // Currently an "undocumented feature".
          // No committees in the main search tab text input.
          if (strpos($search_terms,
              'committee:') === FALSE && $field_name != 'search_committee'
          ) {
            $fields[$field_name] = str_replace(' ', '|', $search_terms);
          }
          // Has committees in the main search tab text input.
          elseif ($field_name != 'search_committee') {
            $pattern = '/(committee:".*?")/';
            preg_match_all($pattern, $search_terms, $matches);

            if (!empty($matches[1])) {
              foreach ($matches[1] as $match) {
                $fields[$field_name] = trim(str_replace($match, '',
                  $fields[$field_name]));
              }

              $non_matches = !empty($fields[$field_name]) ? $fields[$field_name] . '|' : NULL;
              if (!empty($non_matches)) {
                $non_matches = preg_replace('/\s+/', ' ', $non_matches);
                $non_matches = str_replace(' ', '|', $non_matches);
              }

              $committees = implode('|', $matches[1]);

              $fields[$field_name] = $non_matches . $committees;
            }
          }
          // End main tab text input committee parsing.
        }
      }

      $fields['limit'] = $this->pager->options['items_per_page'];
      $fields['offset'] = $this->offset;

      if (isset($this->orderby)) {
        $sort_field = $this->orderby[0]['field'];
        if ($sort_field == 'group_name') {
          $sort_field = 'name';
        }
        $fields['sort'] = $sort_field;
        $fields['direction'] = $this->orderby[0]['direction'];
      }

      // Get the targets.
      // Custom Group search.
      if (isset($fields['class_name']) && $fields['class_name'] == 'Group') {
        $params = $fields;
        unset($params['class_name']);
        $response = $api_call->invokeClientMethod('searchTargetGroups', $params);
      }
      // Legislative and custom individuals.
      else {
        $response = $api_call->invokeClientMethod('searchTargets', $fields);
      }
      $items = array();
      if (isset($response->data->targets)) {
        $items = $response->data->targets;
        foreach ($items as $key => $item) {
          $items[$key]->created_at = date('m/j/Y', strtotime($item->created_at));
          $items[$key]->updated_at = date('m/j/Y', strtotime($item->updated_at));
        }
      }
      elseif (isset($response->data->groups)) {
        // To prevent recursion on group create/edit pages, we will find the
        // parent group ID and filter that result if it is present.
        $parent_group_id = -1;
        if (arg(4) == "custom-groups" && is_numeric(arg(5))) {
          $parent_group_id = arg(5);
        }
        $groups = $response->data->groups;
        foreach ($groups as $key => $item) {
          if ($item->id != $parent_group_id) {
            $items[$item->id] = new stdClass();
            $items[$item->id]->group_name = $item->name;
            $items[$item->id]->id = $item->id;
            $items[$item->id]->matching_targets = sba_target_search_process_matches($item->matching_targets);
            $items[$item->id]->child_groups = sba_target_search_process_groups($item->groups);
            $items[$item->id]->created_at = date('m/j/Y', strtotime($item->created_at));
            $items[$item->id]->updated_at = date('m/j/Y', strtotime($item->updated_at));
          }
        }
      }
      if (!empty($items)) {
        // Build the pager.
        if ($this->pager->use_pager()) {
          $this->pager->total_items = $response->data->count;
          if (!empty($this->pager->options['offset'])) {
            $this->pager->total_items -= $this->pager->options['offset'];
          }
        }
        // Add target IDs to build info. We retrieve elsewhere for use in
        // Target Add links.
        $view->build_info['target_ids'] = $response->data->ids;
      }

      if ($this->pager->use_pager()) {
        $this->pager->update_page_info();
      }

      $view->result = array();

      // Put target data into results array.
      if (!empty($items)) {
        $view->result = $items;
      }

      // Build the pager.
      $this->pager->post_execute($view->result);
      if ($this->pager->use_pager()) {
        $view->total_rows = $this->pager->get_total_items();
      }

      $view->execute_time = microtime(TRUE) - $start;
    }
  }

  /**
   * @param $table
   * @param null $field
   * @param string $order
   * @param string $alias
   * @param array $params
   */
  function add_orderby(
    $table,
    $field = NULL,
    $order = 'ASC',
    $alias = '',
    $params = array()
  ) {
    $this->orderby[] = array(
      'field' => $alias,
      'direction' => strtoupper($order),
    );
  }

}