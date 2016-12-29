/**
 * @file
 * JQuery P2P Donation form.
 */
(function ($) {
  // Add drupal 7 code.
  Drupal.behaviors.p2pDonation = {
    attach: function (context, settings) {
// End drupal calls.

      // Add uniform to selects.
      $('select').once('p2p-select', function () {
        $(this).uniform();
      });

      // set a var for the p2p donation form page type.
      var p2pd = '.p2p-donation-form';

      // Define an instance of jquery mediacheck.
      mediaCheck({
        media: '(max-width: 980px)',
        entry: function() {
          // < 980 sidebar is first.
          $(p2pd  + " " +  '.panel-col-last').insertBefore(p2pd  + " " + '.panel-col-first');
          // Move the ssl cert over to the sidebar when < 980.
          $('#ssl-cert').insertAfter('.fundraiser_submit_message');
        },
        exit: function() {
         // > 980 sidebar is last.
          $(p2pd  + " " +  '.panel-col-first').insertBefore(p2pd  + " " + '.panel-col-last');
          // Move the ssl cert over to the sidebar when > 980.
          $('#ssl-cert').appendTo('.panel-panel.panel-col-last .inside');
        }
      });

      // Add meaningful classes to webform radio buttons & labels.
      $('input[type=checkbox]').addClass('checkbox-input');
      $('input[type=checkbox]').next('label').addClass('checkbox-label');

      $('.webform-client-form input[type="radio"]').each(function() {
        $(this).next("label").addClass($(this).attr("type") + '-' + $(this).attr("value"));
        $(this).addClass('radio-button-input');
        $(this).next("label").addClass('radio-label');
      });

      // checkboxes checked / not checked.
      $('input[type=checkbox]').click(function () {
        if ($(this).is(':checked')) {
          $(this).next('label').toggleClass('checked', this.checked).removeClass('not-checked');
        }
        else {
          $(this).next('label').addClass('not-checked').removeClass('checked');
        }
      });

      // Generic checked / unchecked for radio.
      var radioChecked = $('.single-radio input').is(':checked');
      $('.single-radio input').click(function() {
        radioChecked = !radioChecked;
        $(this).attr('checked', radioChecked);
      });

      // Test for radio onclick.
      var $radioButtons = $('.webform-client-form input[type="radio"]');
      $radioButtons.click(function() {
        $radioButtons.each(function() {
          $(this).next('label').toggleClass('radio-checked', this.checked);
        });
      });

      // Test for which radio checked onload.
      var $radioButtonsLoad = $('.webform-client-form input[type="radio"]');
      $radioButtonsLoad.each(function() {
        $(this).next("label").toggleClass('radio-checked', this.checked);
      });

      // Test if checkbox has a default checked value on load.
      var $checkBoxLoad = $('.webform-client-form input[type="checkbox"]');
      $checkBoxLoad.each(function() {
        $(this).next("label").toggleClass('checked', this.checked);
      });

      // Remove the padlock icon, we replace with font-awesome.
      $('.fundraiser_submit_message img ').remove();

      // Show / hide other amount input box when other amount is clicked / touched.
      $('.webform-client-form .option.radio-other.radio-label').click(function() {
        $('#other-wrapper').css('display', 'inline-block');
      });

      $('.webform-client-form .option.radio-label:not(".radio-other")').click(function () {
        $('#other-wrapper').css('display', 'none');
      });

      // Only on default form output, not on springboard.
      if ($('div.form-item').length && $('.fundraiser-donation-form').length) {
      // Add classes to enhance default forms that don't have bootstrap classes.
        $('.fundraiser-donation-form').validate({
          onfocusout: function (element) {
            $(element).valid();
          },
          highlight: function (element) {
            $(element).closest('.form-item').removeClass('success').addClass('error');
          },
          success: function (element) {
            element.text('OK').addClass('valid').closest('.form-item').removeClass('error').addClass('success');
          }
        });
      }

      // odd even for confirmation page table.
      $(".pane-p2p-confirmation table tr:visible:even").addClass("even");
      $(".pane-p2p-confirmation table tr:visible:odd").addClass("odd");

      // first / last for confirmation table
      $('.pane-p2p-confirmation table tr td:first-child').addClass('first');
      $('.pane-p2p-confirmation table tr td:last-child').addClass('last');

      // end jquery.
    }};
})
  (jQuery);
