(function($) {
  Drupal.behaviors.braintreeApplepay = {
    attach: function(context, settings) {
      settings = settings.braintree;
      settings.applepay.autofill = 'always';

      $(document).on('braintree.methodChange', function(event, paymentMethod) {
        if (paymentMethod == 'applepay') {
          compactApplepay(true);
        }
        else if (paymentMethod == 'credit') {
          compactApplepay(false);
        }
      });

      $(document).on('braintree.autoFilled', function(event, btInstance, payLoad, autofilled) {
        if (btInstance.settings.currentPaymentMethod == 'applepay' && autofilled) {
          populatePhone(payLoad);
          if (!verifyApplepayFields(payLoad)) {
            // Missing required fields, expose them.
            compactApplepay(false);
          }
        }
      });

      var verifyApplepayFields = function(obj) {
        var remoteDonorFields = ['email', 'firstName', 'lastName', 'address', 'state', 'zip'];
        for (var i = 0; i < remoteDonorFields.length; i++) {
          if (!obj.hasOwnProperty(remoteDonorFields[i]) || !obj[remoteDonorFields[i]]) {
            return false;
          }
        }
        return true;
      };

      var populatePhone = function(obj) {
        var phone = null;
        var billingPhone = $('#webform-component-billing-information input[name$="sbp_phone]"]');
        var donorPhone = $('#webform-component-donor-information input[name$="sbp_phone]"]');
        if (billingPhone.length) {
          phone = $(billingPhone);
        }
        if (donorPhone.length) {
          phone = $(donorPhone);
        }
        if (phone && obj.hasOwnProperty('phone') && obj.phone != '') {
          $(phone).val(obj.phone);
        }
      };

      var compactApplepay = function(collapse) {
        if (settings.applepay && settings.applepay.compact && hasDefaultFields()) {
          var fieldsetsVisible = $('#webform-component-donor-information:visible, #webform-component-billing-information:visible').length > 0;
          var currentState = settings.applepay.collapsed;
          var layout = settings.applepay.form_layout || false;
          if (collapse && fieldsetsVisible) {
            if (layout == 'two_column_donation') {
              $('#left').hide();
            }
            $('#webform-component-donor-information').hide();
            $('#webform-component-billing-information').hide();
            settings.applepay.collapsed = true;
            if (settings.paypal !== undefined) {
              settings.paypal.collapsed = true;
            }
          } else if (!collapse && !fieldsetsVisible) {
            if (layout == 'two_column_donation') {
              $('#left').show();
            }
            $('#webform-component-donor-information').show();
            $('#webform-component-billing-information').show();
            settings.applepay.collapsed = false;
            if (settings.paypal !== undefined) {
              settings.paypal.collapsed = false;
            }
          }
        }
      };

      var hasDefaultFields = function() {
        var fundraiserDefaultFields = {
          donorDefaults: [
            'first_name',
            'last_name',
            'mail',
          ],
          billingDefaults: [
            'address',
            'address_line_2',
            'city',
            'country',
            'state',
            'zip',
          ],
        };

        var notHidden = function() {
          return $(this).attr('type') != 'hidden';
        };

        var donorInfo = $('#webform-component-donor-information :input').filter(notHidden);

        var billingInfo = $('#webform-component-billing-information :input').filter(notHidden);
        var onlyDefaults = true;
        $.each(donorInfo, function(key, value) {
          var n = $(value).attr('name').match(/[^[\]]+(?=])/g)[1];
          if ($.inArray(n, fundraiserDefaultFields.donorDefaults) < 0 && n != 'sbp_phone') {
            onlyDefaults = false;
            return;
          }
        });
        $.each(billingInfo, function(key, value) {
          var n = $(value).attr('name').match(/[^[\]]+(?=])/g)[1];
          if ($.inArray(n, fundraiserDefaultFields.billingDefaults) < 0 && n != 'sbp_phone') {
            onlyDefaults = false;
            return;
          }
        });
        return onlyDefaults;
      };
    },
  };
})(jQuery);
