(function ($) {

  // Makes ajax call to social share endpoint clicking share link
  // Grabs social share referrer URL and appends it to link
  // Opens popup social window.
  // Prevents double share due to repreated link clcking or popup blockers.
  Drupal.behaviors.sb_social = {
    attach: function (context, settings) {
      $('.social-share-link').each(function () {
        var link = window.location.href;
        // Drupal once to limit clicks on ajax page reload.
        $(this).once('popup', function() {
          $(this).click(function (e) {
            // Setup some vars we use later.
            var $elem = $(this);
            var $url = Drupal.settings.basePath;
            var service;

            // Build social share ajax URLS.
            if ($(this).hasClass('facebook')) {
              $url += 'sb_social/share_event/facebook/';
              service = 'facebook';
            }
            if ($(this).hasClass('twitter')) {
              $url += 'sb_social/share_event/twitter/';
              service = 'twitter';
            }
            if ($(this).hasClass('email')) {
              $url += 'sb_social/share_event/email/';
              service = 'email';
            }
            $url = $url + Drupal.settings.sb_social.id + '/';
            $url = $url + Drupal.settings.sb_social.id_type + '/';
            $url = $url + Drupal.settings.sb_social.market_source + '/';
            if (typeof Drupal.settings.sb_social.submission_id !== 'undefined') {
              $url = $url + '?sid=' + Drupal.settings.sb_social.submission_id;
            }

            // Make the ajax call to register the share with Drupal (and Salesforce).
            jQuery.ajax({
              beforeSend: function () {
                // Don't do the ajax call if this link has already been clicked
                // and the popup window has been blocked or is still open.
                if ($elem.hasClass('social-processed')) {
                  return false;
                }
              },
              url: $url,
              success: function (response) {
                // The response is a link to the page, with a social
                // share parameter appended.
                // The shorten module has an option to 'www' instead of 'http'
                // which breaks twitter integration. add http to links.
                if (response.indexOf("http") === -1) {
                  response = 'https://' + response;
                }
                link = encodeURIComponent(response);
              },
              // Don't use async because we have to wait for the updated link.
              async: false
            });

            // Mark this link as clicked.
            $(this).addClass('social-processed');

            //Open the popup or email app.
            var settings = Drupal.settings.sb_social;
            switch (service) {
              case 'twitter':
                t = socialPopup(this, 'https://twitter.com/intent/tweet?text=' + settings.twitter_message + '&url=' + link);
                e.returnValue = false;
                e.preventDefault();
                break;
              case  'facebook':
                socialPopup(this, 'https://facebook.com/sharer.php?u=' + link);
                e.returnValue = false;
                e.preventDefault();
                break;
              case 'email':
                $elem.attr('href', ' mailto:?subject=' + settings.email_subject + '&body=' + settings.email_message + "\n\n" + link);
                $(this).removeClass('social-processed');
                break;
            }
          });
        });
      });
    }
  };

  // Popup opener.
  var socialPopup = function (item, path) {

    if (!path) {
      console.log("path must not be empty");
      return false;
    }

    var options = {
      windowName: 'sbSocialPopup',
      windowOptions: 'width=550,height=420',
      callback: function (item) {
        // The callback function gets fired (by setInterval) when the window
        // is closed, thereby re-enabling subsequent ajax calls
        // to get new social share IDs.
        $(item).removeClass('social-processed');
        window.sbSocialPopup = null;
      }
    };

    // If the window already exists, just update the location, otherwise
    // open one.
    if (window.sbSocialPopup) {
      // Firefox won't focus an already open window unless you close it first.
      if (navigator.userAgent.toLowerCase().indexOf('firefox') > -1) {
        window.clearInterval(window.oauthInterval);
        window.sbSocialPopup.close();
        window.sbSocialPopup = window.open(path, options.windowName, options.windowOptions);
      }
      else {
        window.sbSocialPopup.location.href = path;
      }
    }
    else {
      window.sbSocialPopup = window.open(path, options.windowName, options.windowOptions);
    }
    window.sbSocialPopup.focus();

    //If the window is closed, re-enable the social share click event
    //that was previously disabled to prevent double clicks.
    var x = 0;
    window.oauthInterval = window.setInterval(function () {
      x++;
      if ((window.sbSocialPopup && window.sbSocialPopup.closed) || x === 30) {
        window.clearInterval(window.oauthInterval);
        options.callback(item);
      }
    }, 1000, x);
  };

})(jQuery);

