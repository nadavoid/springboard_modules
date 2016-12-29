
  <?php if (variable_get('springboard_social_use_addthis', FALSE)) : ?>
    <a class="addthis_button_<?php print $service; ?> social-share-link <?php print $service; ?>" addthis:url="" <?php print $extra; ?>>
  <?php else: ?>
    <a href="#"  class="sb_social_button_<?php print $service; ?> social-share-link <?php print $service; ?>"><?php if (!empty($contents)) {print $contents;} ?></a>
  <?php endif; ?>
