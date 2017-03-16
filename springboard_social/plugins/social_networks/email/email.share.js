(function ($) {
  Drupal.behaviors.sb_social_email = {
    attach: function (context, settings) {
      // set up custom configuration for email subject and message
      // sets the contents of the [note] email template variable in the AddThis email template.

      if (typeof window.addthis_config !== "undefined") {
        settings = {
          ui_email_note: Drupal.settings.sb_social.email_message
        };
        jQuery.extend(window.addthis_config, settings);
      }
      else {
        window.addthis_config = {
          ui_email_note: Drupal.settings.sb_social.email_message
        };
      }
    }
  };
})(jQuery);