<div class="<?php print $classes; ?>">
  <?php print render($title_prefix); ?>
  <?php if ($title): ?>
  <?php print $title; ?>
  <?php endif; ?>
  <?php print render($title_suffix); ?>
  <?php if ($header): ?>
  <div class="view-header">
    <?php print $header; ?>
  </div>
  <?php endif; ?>
  <div class="buttons-wrapper">
    <?php
    $path = base_path() . 'springboard/taxonomy/fd_designation_categories/add';
    $link = l(t('Create New Category'), $path, array('query' => array('destination' => current_path()), 'attributes' => array('class' => 'button add-button')));
    ?>
    <?php print $link; ?>
  </div>
  <div class="buttons-wrapper">
    <?php
      $path = base_path() . 'springboard/commerce/products/add/fundraiser-designation';
      $link = l(t('Create New Fund Designation'), $path, array('query' => array('destination' => current_path()), 'attributes' => array('class' => 'button add-button')));
    ?>
    <?php print $link; ?>
  </div>

  <?php if ($exposed): ?>
  <div class="view-filters">
    <?php print $exposed; ?>
  </div>
  <?php endif; ?>

  <?php if ($attachment_before): ?>
  <div class="attachment attachment-before">
    <?php print $attachment_before; ?>
  </div>
  <?php endif; ?>

  <?php if ($rows): ?>
  <div class="view-content">
    <?php print $rows; ?>
  </div>
  <?php elseif ($empty): ?>
  <div class="view-empty">
    <?php print $empty; ?>
  </div>
  <?php endif; ?>

  <?php if ($pager): ?>
  <?php print $pager; ?>
  <?php endif; ?>

  <?php if ($attachment_after): ?>
  <div class="attachment attachment-after">
    <?php print $attachment_after; ?>
  </div>
  <?php endif; ?>

  <?php if ($more): ?>
  <?php print $more; ?>
  <?php endif; ?>

  <?php if ($footer): ?>
  <div class="view-footer">
    <?php print $footer; ?>
  </div>
  <?php endif; ?>

  <?php if ($feed_icon): ?>
  <div class="feed-icon">
    <?php print $feed_icon; ?>
  </div>
  <?php endif; ?>

</div><?php /* class view */ ?>