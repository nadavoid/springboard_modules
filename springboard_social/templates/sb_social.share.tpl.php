<?php if (!empty($description)) : ?>
<div class="social_share_description">
<?php print t($description); ?>
</div>
<?php endif; ?>

<?php if (variable_get('springboard_social_use_addthis', FALSE)) : ?>
<div class = "addthis_toolbox addthis_default_style addthis_16x16_style">
  <?php else: ?>
<div class = "sb_social_toolbox sb_social_default_style sb_social_16x16_style">
  <?php endif; ?>
<?php print $buttons ?>
</div>
