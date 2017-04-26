(function($) {

  Drupal.behaviors.braintreePaypal = {
    attach: function(context, settings) {
      settings = settings.braintree;
      settings.paypal.autofill = 'always';
      $(document).on('braintree.methodChange', function(event, paymentMethod) {
        compactPaypal(paymentMethod == 'paypal');
      });
      $(document).on('braintree.autoFilled', function(event, btInstance, payLoad, autofilled) {
        if (btInstance.settings.currentPaymentMethod == 'paypal' && autofilled) {
          populatePhone(payLoad);
          if (verifyPaypalFields(payLoad)) {
            btInstance.$form.submit();
          }
          else {
            // Missing required fields, expose them.
            compactPaypal(false);
          }
        }
      });

      var verifyPaypalFields = function(obj) {
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
        var billingPhone = $("#webform-component-billing-information input[name$='sbp_phone]']");
        var donorPhone = $("#webform-component-donor-information input[name$='sbp_phone]']");
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

      var compactPaypal = function(collapse) {
        if (settings.paypal && settings.paypal.compact && hasDefaultFields()) {
          var currentState = settings.paypal.collapsed;
          var layout = settings.paypal.form_layout || false;
          if (collapse && !settings.applepay.collapsed) {
            if (layout == 'two_column_donation') {
              $('#left').hide();
            }
            $('#webform-component-donor-information').hide();
            $('#webform-component-billing-information').hide();
            settings.paypal.collapsed = true;
            settings.applepay.collapsed = true;
          } else if (!collapse && settings.applepay.collapsed) {
            if (layout == 'two_column_donation') {
              $('#left').show();
            }
            $('#webform-component-donor-information').show();
            $('#webform-component-billing-information').show();
            settings.paypal.collapsed = false;
            settings.applepay.collapsed = false;
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
