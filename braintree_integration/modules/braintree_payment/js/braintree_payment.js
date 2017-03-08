;(function($) {
  var error = function() {
    return !!console && console.error.apply(this, arguments);
  };

  var BraintreePayment = function(settings) {
    this.settings = settings;
    this.$form = $('#' + settings.formId);
    this.$submit = this.$form.find('input[type=submit]');
    this.$nonce = this.$form.find('input[name=payment_method_nonce]');
    this.clientInstance = null;
    this.hostedFieldsInstance = null;
    this.paypalInstance = null;
    this.$amount = this.$form.find('input[name="submitted[donation][amount]"]');
    this.$otherAmount = this.$form.find('input[name="submitted[donation][other_amount]"]');
    this.amount = 0;
    this.deviceDataInstance = null;
    this.$deviceDataInput = $('<input name="device_data" type="hidden"/>');

    var parent = this;
    var processed = false;

    this.hostedFieldsCreated = false;
    this.paypalFieldsCreated = false;

    var paymentMethods = {};

    this.addPaymentMethod = function(id, callbacks) {
      paymentMethods[id] = callbacks;
    };

    this.removePaymentMethod = function(id, inputId) {
      if (undefined == inputId) {
        inputId = id;
      }

      parent.settings.availableMethods.splice($.inArray(id, parent.settings.availableMethods), 1);
      var $input = $('input[name="submitted[payment_information][payment_method]"][value="' + inputId + '"]');
      if ($input.is(':checked')) {
        // If this method is currently checked, check the first payment
        // method instead.
        var $first_payment_method = $input.parent('.form-item').siblings('.form-item').eq(0).find('input[name="submitted[payment_information][payment_method]"]');
        $first_payment_method.attr('checked', 'checked').trigger('change');
        parent.settings.currentPaymentMethod = $first_payment_method.val();
      }
      $input.parent('.form-item').remove();
    };

    this.updateAmount = function(event, checkedOption) {
      if (undefined === checkedOption) {
        var checked = parent.$amount.filter(':checked');
        if (!checked.length) {
          parent.amount = 0;
          return;
        }

        parent.amount = checked.val();
        if (undefined == parent.amount) {
          return;
        }
      }
      else {
        parent.amount = checkedOption;
      }

      if (parent.amount == 'other') {
        parent.$otherAmount.off('blur.updateAmount').on('blur.updateAmount', function() {
          parent.amount = $(this).val();
        });
      }
      else {
        parent.$otherAmount.off('blur.updateAmount');
      }
    };

    /**
     * Initializes the Braintree client.
     */
    this.bootstrap = function() {
      for (paymentMethod in paymentMethods) {
        if (undefined !== paymentMethods[paymentMethod].preboot) {
          paymentMethods[paymentMethod].preboot.call(this);
        }
      }

      // Filter out the unavailable payment methods.
      parent.settings.enabledMethods = $(parent.settings.enabledMethods).filter(function(index, value) {
        return $.inArray(value, parent.settings.availableMethods) >= 0;
      }).get();

      braintree.client.create({
        authorization: this.settings.clientToken
      }, function(error_message, clientInstance) {
        if (error_message) {
          error(error_message);
          return;
        }

        parent.setClientInstance(clientInstance);
        parent.resetDeviceData();
        parent.setCurrentPaymentMethod(parent.settings.currentPaymentMethod);
      });

      parent.updateAmount();
      parent.$amount.on('change', parent.updateAmount);
      parent.$otherAmount.on('focus', function(e) {
        parent.updateAmount(e, 'other');
      });

      for (paymentMethod in paymentMethods) {
        if (undefined !== paymentMethod.postboot) {
          paymentMethod.postboot.call(this);
        }
      }

      return this;
    };

    this.resetDeviceData = function() {
      if (null !== parent.deviceDataInstance) {
        parent.deviceDataInstance.teardown();
      }

      parent.deviceDataInstance = null;
      parent.$deviceDataInput.val('');
      // Add fraud protection.
      if (parent.settings.currentPaymentMethod == 'paypal') {
        braintree.dataCollector.create({
          client: parent.clientInstance,
          paypal: true
        }, function (error_message, dataCollectorInstance) {
          if (error_message) {
            error(error_message);
            return;
          }
          // At this point, you should access the
          // dataCollectorInstance.deviceData value and provide it to your
          // server, e.g. by injecting it into your form as a hidden input.
          parent.deviceDataInstance = dataCollectorInstance;
          parent.$deviceDataInput.val(parent.deviceDataInstance.deviceData);
        });
      }
    }

    this.setCurrentPaymentMethod = function(paymentMethod) {
      $(document).trigger('braintree.methodChange', [paymentMethod]);
      if (parent.settings.currentPaymentMethod == paymentMethod && processed) {
        return this;
      }

      var oldMethod = parent.settings.currentPaymentMethod;

      parent.settings.currentPaymentMethod = paymentMethod;
      for (p in paymentMethods) {
        if (undefined !== paymentMethods[p].disableFieldsSubmit && p != paymentMethod) {
          paymentMethods[p].disableFieldsSubmit.call(this);
        }
      }
      if (paymentMethod == 'credit' && parent.creditEnabled()) {
        // PayPal submission event gets reset in parent.createHostedFields.
        parent.createHostedFields(parent.clientInstance);
      }
      else if (paymentMethod == 'paypal' && parent.paypalEnabled()) {
        parent.createPaypalFields(parent.clientInstance);
        // No need to reset the submission handler for Paypal, as that is done
        // in the `createPaypalFields` function. Just need to disable the hosted
        // fields submission handler.
        parent.disableHostedFieldsSubmit();
      }
      else if (undefined !== paymentMethods[paymentMethod] && paymentMethods[paymentMethod].isEnabled()) {
        parent.disableHostedFieldsSubmit().disablePaypalFieldsSubmit();
        paymentMethods[paymentMethod].createFields(parent.clientInstance);
      }

      parent.resetDeviceData();
      processed = true;
      return this;
    };

    this.setClientInstance = function(clientInstance) {
      this.clientInstance = clientInstance;
      return this;
    };

    /**
     * Helper function to determine if credit payment method is enabled.
     */
    this.creditEnabled = function() {
      return this.settings.enabledMethods.indexOf('credit') >= 0;
    };

    this.enableHostedFieldsEvents = function() {
      parent.hostedFieldsInstance.on('validityChange', function(event) {
        $(document).trigger('braintree.fieldEvent', [event]);
      });
      parent.hostedFieldsInstance.on('inputSubmitRequest', function(event) {
        parent.$submit.click();
      });
      return this;
    };

    this.disableHostedFieldsEvents = function() {
      parent.hostedFieldsInstance.off('validityChange');
      parent.hostedFieldsInstance.off('inputSubmitRequest');
      return this;
    };

    var fEnableHostedFieldsSubmit = false;
    this.enableHostedFieldsSubmit = function() {
      if (fEnableHostedFieldsSubmit) {
        return this;
      }

      // When the form is submitted when the credit payment type is used, we
      // have to wait for an async response from Braintree to determine if the
      // fields validated successfully or not, and to get a token. Therefore, we
      // need to temporarily remove the submit handlers already attached to the
      // form, add ours, and add them back if the form validated correctly.
      var submitHandlers = $.data(parent.$form[0], 'events')['submit'];
      parent.callbacks = $.extend(true, [], submitHandlers);
      var guid = 0; // Some arbitrary number.
      if (undefined !== submitHandlers && submitHandlers.length > 0 && submitHandlers[0]) {
        guid = submitHandlers[0].guid - 1;
      }
      submitHandlers.splice(0, submitHandlers.length, {
        data: null,
        guid: guid,
        handler: parent.submitHostedFields,
        namespace: '',
        origType: 'submit',
        quick: null,
        selector: null,
        type: 'submit'
      });
      fEnableHostedFieldsSubmit = true;

      return this;
    };

    this.disableHostedFieldsSubmit = function() {
      if (fEnableHostedFieldsSubmit) {
        $.data(parent.$form[0], 'events')['submit'] = parent.callbacks;
      }

      fEnableHostedFieldsSubmit = false;
      return this;
    };

    this.resetHostedFieldsSubmit = function() {
      parent.disableHostedFieldsSubmit().enableHostedFieldsSubmit();
      return this;
    };

    this.createHostedFields = function(clientInstance) {
      parent.setClientInstance(clientInstance);
      if (parent.hostedFieldsCreated) {
        return this;
      }

      braintree.hostedFields.create({
        client: parent.clientInstance,
        styles: parent.settings.fieldsStyles,
        fields: parent.settings.hostedFields
      }, function(error_message, hostedFieldsInstance) {
        if (error_message) {
          error(error_message);
          return;
        }
        parent.hostedFieldsInstance = hostedFieldsInstance;
        if (parent.settings.currentPaymentMethod == 'paypal') {
          parent.disablePaypalFieldsSubmit();
        }
        else if (undefined !== paymentMethods[parent.settings.currentPaymentMethod]
          && paymentMethods[parent.settings.currentPaymentMethod].isEnabled()
          && undefined !== paymentMethods[paymentMethod].disableFieldsSubmit
        ) {
          paymentMethods[paymentMethod].disableFieldsSubmit.call(this);
        }
        parent.resetHostedFieldsSubmit();
        parent.enableHostedFieldsEvents();
        parent.hostedFieldsCreated = true;
      });

      return this;
    };

    this.submitHostedFields = function(event) {
      if (parent.settings.currentPaymentMethod != 'credit') {
        return;
      }

      var isBillingUpdateForm = undefined !== parent.settings.billing_update_type;

      if (!parent.$nonce.val().length) {
        event.preventDefault();
        var triggerBraintreeFieldEvent = function() {
          var state = parent.hostedFieldsInstance.getState();
          for (field in state.fields) {
            state.emittedBy = field;
            $(document).trigger('braintree.fieldEvent', [state]);
          }
        };
        parent.hostedFieldsInstance.tokenize(function(error_message, payload) {
          // Show errors when not on a billing update form, or when on a billing
          // update form and only some fields are filled out.
          if (!isBillingUpdateForm && error_message || (isBillingUpdateForm && error_message && error_message.code != 'HOSTED_FIELDS_FIELDS_EMPTY')) {
            switch (error_message.code) {
              case 'HOSTED_FIELDS_FIELDS_EMPTY':
                triggerBraintreeFieldEvent();
                error('All fields are empty! Please fill out the form.');
                break;
              case 'HOSTED_FIELDS_FIELDS_INVALID':
                triggerBraintreeFieldEvent();
                error('Some fields are invalid:', error_message.details.invalidFieldKeys);
                break;
              case 'HOSTED_FIELDS_FAILED_TOKENIZATION':
                error('Tokenization failed server side. Is the card valid?');
                break;
              case 'HOSTED_FIELDS_TOKENIZATION_NETWORK_ERROR':
                error('Network error occurred when tokenizing.');
                break;
              default:
                error('Something bad happened!', error_message);
            }
            parent.$submit.show().next('.donation-processing-wrapper').remove();
          }

          if (undefined !== payload && undefined !== payload.nonce) {
            parent.$nonce.val(payload.nonce);
          }

          $.data(parent.$form[0], 'events')['submit'] = parent.callbacks;
          parent.$form.submit();
        });
      }

      return this;
    };

    this.paypalEnabled = function() {
      return this.settings.enabledMethods.indexOf('paypal') >= 0;
    };

    this.setPaypalInstance = function(paypalInstance) {
      this.paypalInstance = paypalInstance;
      return this;
    };

    this.enablePaypalEvents = function() {
      parent.paypalInstance.on('validityChange', function(event) {

      });
      return this;
    };

    this.disablePaypalEvents = function() {
      parent.paypalInstance.off('validityChange');
      return this;
    }

    var fEnablePaypalFieldsSubmit = false;
    this.enablePaypalFieldsSubmit = function() {
      if (fEnablePaypalFieldsSubmit) {
        return this;
      }

      // When autofill is enabled, we only want to validate the amount field.
      // Since the donation validation submission handler is already on the
      // queue, we need to insert our submission handler function before the
      // donation validation.
      parent.callbacks = $.extend(true, [], $.data(parent.$form[0], 'events')['submit']);
      var guid = 128;
      if (undefined !== parent.callbacks && parent.callbacks.length) {
        guid = parent.callbacks[0].guid - 1;
      }
      $.data(parent.$form[0], 'events')['submit'].splice(0, parent.callbacks.length, {
        data: null,
        guid: guid,
        handler: parent.submitPaypalFields,
        namespace: '',
        origType: 'submit',
        quick: null,
        selector: null,
        type: 'submit'
      });
      fEnablePaypalFieldsSubmit = true;

      return this;
    };

    this.disablePaypalFieldsSubmit = function() {
      if (fEnablePaypalFieldsSubmit) {
        $.data(parent.$form[0], 'events')['submit'] = parent.callbacks;
      }

      fEnablePaypalFieldsSubmit = false;
      return this;
    };

    this.resetPaypalFieldsSubmit = function() {
      parent.disablePaypalFieldsSubmit().enablePaypalFieldsSubmit();
      return this;
    };

    this.createPaypalFields = function(clientInstance) {
      parent.setClientInstance(clientInstance);
      if (parent.paypalFieldsCreated) {
        return this;
      }

      braintree.paypal.create({
        client: this.clientInstance
      }, function (error_message, paypalInstance) {
        if (error_message) {
          error(error_message);
          return;
        }

        parent.setPaypalInstance(paypalInstance);
        parent.resetPaypalFieldsSubmit();
        parent.paypalFieldsCreated = true;
      });

      return this;
    };

    this.submitPaypalFields = function(event) {
      if (parent.settings.currentPaymentMethod != 'paypal') {
        return;
      }

      if (Drupal.settings.fundraiser.donationValidate.element('#edit-submitted-donation-other-amount') === false) {
        $('#edit-submitted-donation-other-amount').focus();
        event.stopImmediatePropagation();
        event.preventDefault();
        return;
      }

      if (!parent.$nonce.val().length) {
        event.preventDefault();
        parent.paypalInstance.tokenize({
          flow: 'vault',
          amount: parent.amount,
          currency: 'USD',
          locale: 'en_us',
          enableShippingAddress: false
        }, function (error_message, payload) {
          if (error_message) {
            error(error_message);
            return;
          }

          // Tokenization succeeded!
          parent.$nonce.val(payload.nonce);
          $('#braintree-paypal-loggedin').show();
          $('#bt-pp-email').text(payload.details.email);

          // Bind cancel button to restore PayPal form.
          $('#bt-pp-cancel').on('click', function(event) {
            event.preventDefault();
            parent.reset();
          });

          var autofill = parent.settings.paypal.autofill;
          var autofilled = false;
          if (autofill != 'never') {
            var address = {
              firstName: payload.details.firstName,
              lastName: payload.details.lastName,
              email: payload.details.email,
              address: payload.details.billingAddress.line1,
              address2: payload.details.billingAddress.line2,
              city: payload.details.billingAddress.city,
              country: payload.details.billingAddress.countryCode,
              state: payload.details.billingAddress.state,
              zip: payload.details.billingAddress.postalCode,
              phone: payload.details.phone
            };
            autofilled = parent.autofill(address, autofill);
          }

          $.data(parent.$form[0], 'events')['submit'] = parent.callbacks;
          
          $(document).trigger('braintree.autoFilled', [parent, address, autofilled]);

          // Auto-submit the form if no fields were auto-filled from the values
          // in the payload object.
          if (!autofill || (autofill && !autofilled)) {
            parent.$form.submit();
          }
        });
      }
    };

    this.reset = function() {
      parent.$nonce.val('');

      // Reset submit button.
      this.$submit.removeAttr('disabled');

      // Remove device_data if added.
      var deviceDataInput = this.$form[0]['device_data'];
      if (undefined !== deviceDataInput) {
        this.$form.removeChild(deviceDataInput);
      }

      if (this.settings.currentPaymentMethod == 'paypal') {
        $('#braintree-paypal-loggedin').hide();
        $('#bt-pp-email').text('');
      }

      return this;
    };

    this.autofill = function(obj, autofill) {
      var fieldsHaveBeenAutoFilled = false;
      var field_mapping = {
        'submitted[donor_information][first_name]': obj.firstName,
        'submitted[donor_information][last_name]': obj.lastName,
        'submitted[donor_information][mail]': obj.email,
        'submitted[billing_information][address]': obj.address,
        'submitted[billing_information][address_line_2]': obj.address2,
        'submitted[billing_information][city]': obj.city,
        'submitted[billing_information][country]': obj.country,
        'submitted[billing_information][state]': obj.state,
        'submitted[billing_information][zip]': obj.zip,
      };

      // If this flag is false when we examine the secondary address field, then
      // the secondary address field shouldn't be autofilled.
      var primaryAddressMatches = false;
      if ($('[name="submitted[billing_information][address]"]').val() == field_mapping['submitted[billing_information][address]']) {
        primaryAddressMatches = true;
      }

      $.each(field_mapping, function(key, value) {
        var $field = $('[name="' + key + '"]');
        if (autofill == 'always'
          || (autofill == 'if_blank' && $field.val() == '' && key != 'submitted[billing_information][address_line_2]')
          || (key == 'submitted[billing_information][address_line_2]' && autofill == 'if_blank' && primaryAddressMatches && $field.val() == '')) {
          $field.val(value);
          fieldsHaveBeenAutoFilled = true;
        }
      });
      return fieldsHaveBeenAutoFilled;
    }

    // Allow other modules to add their own payment methods.
    $(document).trigger('BraintreePaymentInstance', [this, error]);

    return this.bootstrap();
  };

  Drupal.behaviors.braintree = {
    attach: function(context, settings) {
      settings = settings.braintree;

      // If the $paymentMethod variable is defined, we're probably on a donation
      // page. Otherwise we're probably on a billing update page.
      var $paymentMethod = $('input[name="submitted[payment_information][payment_method]"]');
      if ($paymentMethod.length) {
        if ($paymentMethod.is('[type=hidden]')) {
          settings.currentPaymentMethod = $paymentMethod.val();
        }
        else {
          settings.currentPaymentMethod = $paymentMethod.filter(':checked').val();
        }

        settings.availableMethods = $paymentMethod.map(function() {
          return this.value;
        }).get();
      }
      else if (undefined !== Drupal.settings.braintree.billing_update_type) {
        settings.currentPaymentMethod = Drupal.settings.braintree.billing_update_type;
        settings.availableMethods = [Drupal.settings.braintree.billing_update_type];
        if (!$.inArray(Drupal.settings.braintree.billing_update_type, settings.enabledMethods)) {
          settings.enabledMethods.push(Drupal.settings.braintree.billing_update_type);
        }
      }

      if (undefined !== settings.currentPaymentMethod) {
        Drupal.braintreeInstance = new BraintreePayment(settings);
        $paymentMethod.on('change', function() {
          Drupal.braintreeInstance.setCurrentPaymentMethod($(this).val());
        });
      }
    }
  };
})(jQuery);
