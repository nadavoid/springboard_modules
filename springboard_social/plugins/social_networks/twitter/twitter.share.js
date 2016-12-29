(function ($) {
  Drupal.behaviors.sb_social_twitter = {
    attach: function (context, settings) {
      // set up custom configuration for twitter message
      if (typeof window.addthis_share !== "undefined") {
        settings = {
          passthrough: {
            twitter: {
              text: Drupal.settings.sb_social.twitter_message
            }
          }
        };
        jQuery.extend(window.addthis_share, settings);
      }
      else {
        window.addthis_share = {
          passthrough: {
            twitter: {
              text: Drupal.settings.sb_social.twitter_message
            }
          },
        };
      }
    }
  };
})(jQuery);
