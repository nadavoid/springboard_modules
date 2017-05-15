(function($) {
  Drupal.behaviors.fundraiser_ocd = {
    attach: function(context, settings) {
      // Return early if no payment processors are available.
      if (undefined == Drupal.settings.fundraiser_ocd) {
        return;
      }

      var $gateways = {
        'credit': $('#edit-gateways-credit-id'),
        'bank': $('#edit-gateways-bank-account-id'),
        'paypal': $('#edit-gateways-paypal-id')
      };

      var $gateways_enabled;
      var $ocd = $('.form-item-ocd');
      var gateway_available = {'credit': true, 'bank': false, 'paypal': false};

      var gateways_enabled = function() {
        $gateways_enabled = {
          'credit': $('#edit-gateways-credit-status').is(':checked'),
          'bank': $('#edit-gateways-bank-account-status').is(':checked'),
          'paypal': $('#edit-gateways-paypal-status').is(':checked'),
        };
      };
      gateways_enabled();

      var disabled = false;
      var wasChecked = $ocd.find('input[type=checkbox].ocd').is(':checked');
      var checkGateway = function($this, type) {
        gateways_enabled();

        var $selected = $gateways[type].find('option:selected');
        var $option_name = $selected.text();
        var $option_value = $selected.val();
        var $pleasenote = $ocd.next('.pleasenote');
        var $note = $pleasenote.next('.note');
        if (!$pleasenote.length) {
          $pleasenote = $('<div/>', {
            class: 'pleasenote'
          }).html('<strong>Please note</strong>, donors will not be able to opt-in to One Click Donate when using a bank account or PayPal.');
          $ocd.after($pleasenote);
        }
        if ($gateways_enabled.bank || $gateways_enabled.paypal) {
          $pleasenote.show();
        }
        else {
          $pleasenote.hide();
        }
        if (!$note.length) {
          $note = $('<div/>', {
            class: 'note'
          }).html('Please select a compatible credit gateway in order to enable One Click Donate.');
          $pleasenote.after($note);
        }

        if (type == 'credit') {
          if (!$gateways_enabled[type] || $gateways_enabled[type] && Drupal.settings.fundraiser_ocd.usable_paypment_processors[type].indexOf($option_value) < 0) {
            gateway_available[type] = false;
          }
          else if ($gateways_enabled[type] && Drupal.settings.fundraiser_ocd.usable_paypment_processors[type].indexOf($option_value) >= 0) {
            gateway_available[type] = true;
          }
        }

        if (!$gateways_enabled.credit || ($gateways_enabled.credit && !gateway_available.credit)) {
          $ocd.find('input[type=checkbox].ocd').attr('checked', false).attr('disabled', 'disabled');
          $ocd.nextAll('.form-item.ocd, .form-wrapper.ocd').hide();
          $('#edit-ocd-message-container').hide();
          $note.show();
        }
        else {
          $ocd.find('input[type=checkbox].ocd').removeAttr('disabled');
          if (wasChecked) {
            $ocd.find('input[type=checkbox].ocd').attr('checked', wasChecked);
          }
          $ocd.nextAll('.form-item.ocd, .form-wrapper.ocd').show();
          $('#edit-ocd-message-container').show();
          $note.hide();
        }

        if (!$('#edit-ocd').is(':checked')) {
          $ocd.nextAll('.form-item.ocd, .form-wrapper.ocd').hide();
          $('#edit-ocd-message-container').hide();
        }
      };

      $ocd.find('input[type=checkbox].ocd').on('change', function() {
        wasChecked = $(this).is(':checked');
      });

      $gateways['credit'].add('#edit-gateways-credit-status').add('#edit-gateways-credit-id').on('change', function() {
        checkGateway(this, 'credit');
      });
      if ($gateways_enabled.credit) {
        checkGateway(this, 'credit');
      }

      $gateways['paypal'].add('#edit-gateways-paypal-status').on('change', function() {
        checkGateway(this, 'paypal');
      });
      if ($gateways_enabled.paypal) {
        checkGateway(this, 'paypal');
      }

      $gateways['bank'].add('#edit-gateways-bank-account-status').on('change', function() {
        checkGateway(this, 'bank');
      });
      if ($gateways_enabled.bank) {
        checkGateway(this, 'bank');
      }
    }
  };
})(jQuery);
