


<?php /* Start the count. */ $zebra = 0; ?>

<?php foreach ($types as $type) : ?>

  <?php
  // Define odd / even for use in the definition list.
  $zebra_class = ($zebra % 2) ? 'even' : 'odd';
  ?>

  <div class="types-wrapper <?php print $zebra_class; ?>">
    <h2 class="types"><?php print $type->name; ?></h2>

    <div class="buttons-wrapper">

      <!-- bootstrap drop list widget -->
      <?php if (user_access('create email_wrapper content') || user_access('create page_wrapper content')) : ?>
      <div class="btn-group">
        <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown">
          <?php print t('Options'); ?>
          <span class="caret"></span>
        </button>

        <?php // @todo make the menu below dynamic or contextual? ?>
        <ul class="dropdown-menu views-widget-dropdown">
          <li><a href="<?php print base_path(); ?>admin/springboard/asset-library"><?php print t('Templates & Wrappers'); ?></a></li>
        </ul>
      </div><!--// btn-group -->
      <?php endif; ?>

      <?php if (user_access('create ' . $type->type . ' content')) : ?>
        <a href="<?php print base_path(); ?>node/add/<?php print preg_replace('/_/', '-', $type->type); ?>" class="button add-button"><?php print t('Create'); ?> <?php print $type->name; ?></a>
      <?php endif; ?>
    </div><!-- // buttons-wrapper -->

    <?php print($tables[$type->type]); ?>
    <?php $zebra++; ?>
    <a class="button more-button" href="<?php print base_path(); ?>admin/springboard/<?php print ($springboard_type == 'fundraiser') ? 'donation-forms' : 'forms'; ?>/<?php print $type->type; ?>">
      <?php print t('View all');?> <?php print $type->name; ?><?php print t('s');?> </a>

  </div>

<?php endforeach; ?>

