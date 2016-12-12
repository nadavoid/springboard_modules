<?php print $tabs; ?>
<h2>Message Delivery Overview</h2>
<div class="container deliverability-performance-container">
  <div class="row">
    <div class="span3">
      <div class="well">
        <h2 class="text-info"><?php print $total_messages_processed; ?></h2>
        Processed Messages
      </div>
    </div>
    <div class="span3">
      <div class="well">
        <h2 class="text-info"><?php print $delivered; ?></h2>
        Delivered
      </div>
    </div>
    <div class="span3">
      <div class="well">
        <h2 class="text-info"><?php print $undeliverable; ?></h2>
        Undeliverable
      </div>
    </div>
    <div class="span3">
      <div class="well">
        <h2 class="text-info"><?php print $percentage; ?></h2>
        Deliverability
      </div>
    </div>
  </div>
  <div class="row">
    <div class="span12 messages-generated">
      <?php print $total_messages_generated; ?> messages generated as of <?php print date('m/d/y g:ia T',$deliverability_cache_timestamp); ?>
    </div>
  </div>
</div>
<h2>Legislative Target Results</h2>
  <div class="download-csv-results feed-icon-wrapper">
    <a href="<?php print $legislative_results_download_link; ?>" target="_blank"
       class="views-data-export"><span class="img-caption">Download as
      .CSV</span></a>
  </div>
<div class="advocacy-targets-select">
  <form id="sba-legislative-results-pager" method="get"
        action="/springboard/<?php print
    $form_url_with_input;?>">
    <label for="">Results per Page</label>
    <!--<input type="hidden" name="sba-legislative-results-pager-params" />-->
    <select name="limit">
      <option value="25" <?php sba_message_action_pager_option_selected(50); ?>
        >25</option>
      <option value="50" <?php sba_message_action_pager_option_selected(50); ?>
>50</option>
      <option value="100" <?php sba_message_action_pager_option_selected(100);
      ?>>100</option>
      <option value="250" <?php sba_message_action_pager_option_selected(250)
      ; ?>>250</option>
      <option value="500" <?php sba_message_action_pager_option_selected(500)
      ; ?>>500</option>
      <option value="all" <?php sba_message_action_pager_option_selected('all')
      ; ?>>All</option>
    </select>
  </form>
</div>
<?php print $targets_table; ?>