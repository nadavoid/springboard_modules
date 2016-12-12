<?php
/**
 * @file
 * Template file to customize a fundraiser's expiration message.
 *
 * Available variables:
 * - $message: The expiration message.
 */
?>

<?php if ($message): ?>
  <div class="expiration-message">
    <?php print $message; ?>
  </div>
<?php endif; ?>
