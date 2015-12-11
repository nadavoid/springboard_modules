(function ($) {
  Drupal.behaviors.socialCounter = {
    attach: function (context, settings) {
      window.sbaCountable = {};
      Drupal.settings.charCount = {size: 0, count: 0, person: ''};
      sbaCountable.charCount = function () {
        var currentCount = Drupal.settings.charCount.size;
        var message = 'This option has the potential to cause hundreds of targets to be presented to the user. Check back here when updating your target sets to get a count of all possible targets.';
        if (Drupal.settings.charCount.count > 0) {
          message = 'This message currently has ' + Drupal.settings.charCount.count + ' eligible targets.  The allowed maximum is 25.';
        }
        var charMess = 'You have used <span class="counter"></span> characters in your default message. You currently have a maximum of <span class="counter-max"></span> characters for this message.<br /><br />';
        var districted = $("input[name*=field_sba_target_option]:checked").val();
        if (districted == 1) {
          $('.sba-target-status').hide('slow');
        }
        $('.undistricted-update').hide().text(message).show('slow');
        if(Drupal.settings.charCount.count > 0) {
          message += '<br /><br />'
          var alreadyshow = $('.sba-target-status').text().length;
          if (alreadyshow > 0) {
            if (districted != 1) {
              $('.sba-target-status').html(message).show('slow');
            }
            $('.sba-charcount-status').html(charMess).show('slow');
          }
          else {
            $('.sba-charcount-status').hide().html(charMess).show('slow');
            if (districted != 1) {
              $('.sba-target-status').hide().html(message).show('slow');
            }
          }
        }
        else {
          $('.sba-target-status').text('');
        }

        $text = $('div[id*="edit-messages"]').find('textarea');
        var direction = 'down';
        var strict = true;
        if ($text.length == 0) {
          direction = 'up';
          $text = $('#edit-field-sba-twitter-message-und-0-value');
          strict = false;
        }

        $text.each(function () {
          if (this.id = 'edit-field-sba-twitter-message-und-0-value') {
            //message edit page
            var handleCount = sbaCheckMax(this);
            var target = $('body').find('.counter');
          }
          else {
            // message preview page
            handleCount = $(this).closest('.message-preview-message-fieldset').prev('.message-preview-header').find('.twitter-handle').text().length + 1;
            var target = $(this).siblings('.description').find('.counter');

          }
           $('#' + this.id).simplyCountable({
            counter: target,
            countType: 'characters',
            maxCount: 140 - handleCount,
            strictMax: strict,
            countDirection: direction,
            safeClass: 'safe',
            overClass: 'over',
            thousandSeparator: ',',
            onOverCount: function (count, countable, counter) {
              $('#edit-field-sba-twitter-message-und-0-value').addClass('twit-too-long');
            },
            onSafeCount: function (count, countable, counter) {
              $('#edit-field-sba-twitter-message-und-0-value').removeClass('twit-too-long');
            },
            onMaxCount: function (count, countable, counter) {
            }
          });
        })
      };
    }
  };

  $(document).ready(function(){
    $('input[name*="field_sba_prepend_target_name"]').on('click', function() {
      sbaCheckMax($('#edit-field-sba-twitter-message-und-0-value'));
    });
    $('input[name*="field_sba_target_option"]').on('change', function() {
      sbaCountable.charCount();
    });
  });

  var sbaCheckMax = function (editText) {
    var prepend = $('input[name*="field_sba_prepend_target_name"]').attr('checked');
    var period = 0;
    if(typeof prepend != 'undefined') {
      period = 1;
    }
    var handleCount = Drupal.settings.charCount.size;
    if (handleCount != 0) {
      handleCount = handleCount + 1 + period
    }
    $(editText).siblings('.description').find('.counter-max').text(140 - handleCount);
    $('.sba-target-status').find('.counter-max').text(140 - handleCount);
    $('.sba-charcount-status').find('.counter-max').text(140 - handleCount);

    var currLen = $(editText).val().replace(/(\r\n|\n|\r)/gm, "").length;
    if (currLen > 140 - handleCount) {
      $(editText).addClass('error');
    }
    else {
      $(editText).removeClass('error');
    }
    return handleCount;
  }

})(jQuery);