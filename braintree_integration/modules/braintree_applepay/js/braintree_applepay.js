;(function($) {
  $(document).on('BraintreePaymentInstance', function(e, BraintreePaymentInstance, error) {
    BraintreePaymentInstance.addPaymentMethod('applepay', (function() {
      var BIAP = {
        applePayInstance: null
      };
      var BI = BraintreePaymentInstance;

      BIAP.preboot = function() {
        // If Apple Pay is enabled, check if the browser supports it.
        if ($.inArray('applepay', BI.settings.enabledMethods) !== false) {
          var supports_applepay = true;
          if (!window.ApplePaySession) {
            error('This device does not support Apple Pay.');
            supports_applepay = false;
          }
          if (supports_applepay && !ApplePaySession.canMakePayments()) {
            error('This device is not capable of making Apple Pay payments.');
            supports_applepay = false;
          }
          if (!supports_applepay) {
            // Remove the Apple Pay option.
            BI.removePaymentMethod('applepay');
          }
        }
      };

      BIAP.isEnabled = function() {
        return BI.settings.enabledMethods.indexOf('applepay') >= 0;
      };

      BIAP.setInstance = function(applePayInstance) {
        BIAP.applePayInstance = applePayInstance;
        return BIAP;
      };

      var fEnableFieldsSubmit = false;
      BIAP.enableFieldsSubmit = function() {
        // When autofill is enabled, we only want to validate the amount field.
        // Since the donation validation submission handler is already on the
        // queue, we need to insert our submission handler function before the
        // donation validation.
        BIAP.callbacks = $.extend(true, [], $.data(BI.$form[0], 'events')['submit']);
        var guid = 128;
        if (undefined !== BIAP.callbacks && BIAP.callbacks.length) {
          guid = BIAP.callbacks[0].guid - 1;
        }
        $.data(BI.$form[0], 'events')['submit'].splice(0, BIAP.callbacks.length, {
          data: null,
          guid: guid,
          handler: BIAP.submitFields,
          namespace: '',
          origType: 'submit',
          quick: null,
          selector: null,
          type: 'submit'
        });
        fEnableFieldsSubmit = true;
        return BIAP;
      };

      BIAP.disableFieldsSubmit = function() {
        if (fEnableFieldsSubmit) {
          $.data(BI.$form[0], 'events')['submit'] = BIAP.callbacks;
        }

        fEnableFieldsSubmit = false;
        return BIAP;
      };

      BIAP.resetFieldsSubmit = function() {
        BIAP.disableFieldsSubmit().enableFieldsSubmit();
        return BIAP;
      };

      BIAP.submitFields = function(event) {
        if (BI.settings.currentPaymentMethod != 'applepay') {
          return;
        }

        if (Drupal.settings.fundraiser.donationValidate.element('#edit-submitted-donation-other-amount') === false) {
          $('#edit-submitted-donation-other-amount').focus();
          event.stopImmediatePropagation();
          event.preventDefault();
          return;
        }

        if (!BI.$nonce.val().length) {
          event.preventDefault();
          var paymentRequest = BIAP.applePayInstance.createPaymentRequest({
            total: {
              label: BI.settings.storeName,
              amount: BI.amount
            },
            requiredBillingContactFields: ['postalAddress', 'name'],
            requiredShippingContactFields: ['email']
          });

          var session = new ApplePaySession(1, paymentRequest);

          session.onvalidatemerchant = function(event) {
            BIAP.applePayInstance.performValidation({
              validationURL: event.validationURL,
              displayName: BI.settings.storeName
            }, function (validationErr, merchantSession) {
              if (validationErr) {
                // You should show an error to the user, e.g. 'Apple Pay failed to
                // load.'
                error('Error validating merchant:', validationErr);
                session.abort();
                return;
              }
              session.completeMerchantValidation(merchantSession);
            });
          };

          session.onpaymentauthorized = function (event) {
            BIAP.applePayInstance.tokenize({
              token: event.payment.token
            }, function(tokenizeErr, payload) {
              if (tokenizeErr) {
                error('Error tokenizing Apple Pay:', tokenizeErr);
                session.completePayment(ApplePaySession.STATUS_FAILURE);
                return;
              }

              session.completePayment(ApplePaySession.STATUS_SUCCESS);
              BI.$nonce.val(payload.nonce);
              $.data(BI.$form[0], 'events')['submit'] = BIAP.callbacks;

              var autofill = BI.settings.applepay.autofill;
              if (autofill != 'never') {
                var autofilled = BI.autofill({
                  firstName: event.payment.billingContact.givenName,
                  lastName: event.payment.billingContact.familyName,
                  email: event.payment.shippingContact.emailAddress,
                  address: event.payment.billingContact.addressLines[0],
                  address2: undefined == event.payment.billingContact.addressLines[1] ? '' : event.payment.billingContact.addressLines[1],
                  city: event.payment.billingContact.locality,
                  country: event.payment.billingContact.countryCode.toUpperCase(),
                  state: event.payment.billingContact.administrativeArea,
                  zip: event.payment.billingContact.postalCode
                }, autofill);
              }

              BI.$form.submit();
              // Otherwise the user will need to press submit again.
            });
          };

          session.begin();
        }
      };

      BIAP.createFields = function(clientInstance) {
        BI.setClientInstance(clientInstance);
        if (BIAP.applepayFieldsCreated) {
          return BIAP;
        }

        braintree.applePay.create({
          client: BI.clientInstance
        }, function (error_message, applePayInstance) {
          if (error_message) {
            error(error_message);
            BI.removePaymentMethod('applepay');
            return;
          }

          BIAP.setInstance(applePayInstance);

          var promise = ApplePaySession.canMakePaymentsWithActiveCard(applePayInstance.merchantIdentifier);
          promise.then(function(canMakePaymentsWithActiveCard) {
            if (canMakePaymentsWithActiveCard) {
              BI.disableHostedFieldsSubmit().disablePaypalFieldsSubmit();
              BIAP.resetFieldsSubmit();
              BIAP.applepayFieldsCreated = true;
            }
            else {
              BI.removePaymentMethod('applepay');
            }
          });
        });
        return BIAP;
      };

      return BIAP;
    })());
  });
})(jQuery);
