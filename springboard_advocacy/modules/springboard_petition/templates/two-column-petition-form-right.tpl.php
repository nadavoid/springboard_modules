<?php
/**
 * @file
 * This template renders a petition enabled node in 2 columns.
 */

?>

<div id="webform-form-column-wrapper" class="form-layout-container form-right">
  <div class="form-row">

    <div id="left" class="form-col">
      <h1 class="form-layout-title"><?php print render($element['title']); ?></h1>
      <?php print render($element['body']); ?>
      <?php print isset($recent_signers_opt_in_view) ? $recent_signers_opt_in_view : ''; ?>
    </div>

    <div id="right" class="form-col">
      <div id="webform-form" class="form-wrapper">
        <div class="form-wrapper-inner">
          <?php print render($element['webform']); ?>
        </div>
      </div>
      <div id="petition-quick-sign" class="form-wrapper">
        <div class="form-wrapper-inner">
          <?php print render($element['sba_quicksign']); ?>
        </div>
      </div>
    </div>
  </div>
</div>
