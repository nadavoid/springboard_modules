(function($) {
  Drupal.behaviors.fundraiser_quick_donate = {
    attach: function(context, settings) {
      // Return early if no payment processors are available.
      if (undefined == Drupal.settings.fundraiser_quick_donate) {
        return;
      }

      var $gateways = {
        'credit': $('#edit-gateways-credit-id'),
        'bank': $('#edit-gateways-bank-account-id'),
        'paypal': $('#edit-gateways-paypal-id')
      };

      var $gateways_enabled;
      var $quickdonate = $('.form-item-quickdonate');
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
      var wasChecked = $quickdonate.find('input[type=checkbox]').is(':checked');
      var checkGateway = function($this, type) {
        gateways_enabled();

        var $selected = $gateways[type].find('option:selected');
        var $option_name = $selected.text();
        var $option_value = $selected.val();
        var $pleasenote = $quickdonate.next('.pleasenote');
        var $note = $pleasenote.next('.note');
        if (!$pleasenote.length) {
          $pleasenote = $('<div/>', {
            class: 'pleasenote'
          }).html('<strong>Please note</strong>, donors will not be able to opt-in to Quick Donate when using a bank account or PayPal.');
          $quickdonate.after($pleasenote);
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
          }).html('Please select a compatible credit gateway in order to enable Quick Donate.');
          $pleasenote.after($note);
        }

        if (type == 'credit') {
          if (!$gateways_enabled[type] || $gateways_enabled[type] && Drupal.settings.fundraiser_quick_donate.usable_paypment_processors[type].indexOf($option_value) < 0) {
            gateway_available[type] = false;
          }
          else if ($gateways_enabled[type] && Drupal.settings.fundraiser_quick_donate.usable_paypment_processors[type].indexOf($option_value) >= 0) {
            gateway_available[type] = true;
          }
        }

        if (!$gateways_enabled.credit || ($gateways_enabled.credit && !gateway_available.credit)) {
          $quickdonate.find('input[type=checkbox]').attr('disabled', 'disabled');
          $quickdonate.nextAll('.form-item, .form-wrapper').hide();
          $('#edit-quickdonate-message-container').hide();
          $note.show();
        }
        else {
          $quickdonate.find('input[type=checkbox]').removeAttr('disabled');
          if (wasChecked) {
            $quickdonate.find('input[type=checkbox]').attr('checked', wasChecked);
          }
          $quickdonate.nextAll('.form-item, .form-wrapper').show();
          $('#edit-quickdonate-message-container').show();
          $note.hide();
        }

        if (!$('#edit-quickdonate').is(':checked')) {
          $quickdonate.nextAll('.form-item, .form-wrapper').hide();
          $('#edit-quickdonate-message-container').hide();
        }
      };

      $quickdonate.find('input[type=checkbox]').on('change', function() {
        if ($(this).is(':checked')) {
          wasChecked = true;
        }
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
