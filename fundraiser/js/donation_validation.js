(function($) {
  Drupal.behaviors.springboardForms = {
    attach: function (context, settings) {
      var clearElement = function($selector) {
        $selector
          .val('')
          .removeClass('valid')
          .next('label')
            .remove()
            .end()
          .parents('.success')
            .removeClass('success')
            .end()
          .parents('.error')
            .removeClass('error')
        ;
      }

      $(document).ready(function() {
        // Turn autocomplete off on CC and CVV form elements.
        $('input[name*="card_number"], input[name*="card_cvv"]').attr('autocomplete', 'off');

        // Helper function, provides the total display.
        function _recalculate_quantity_total() {
          if (!$('select[name*="quantity"]').length) {
            return;
          }

          $('#quantity-total').empty();
          var amount = $('input[type="radio"][name*="amount"]:checked:visible').val();
          if (amount == 'other') {
            amount = $('input[name*="other_amount"]').val();
          }
          // Prevent total from displaying NaN if other amount input is
          // incorrectly formatted.
          var total = 0.00;
          if (!isNaN(amount)) {
            var total = $('select[name*="quantity"]').val() * amount;
          }
          $('select[name*="quantity"]').after('<span id="quantity-total">Total: ' + Drupal.settings.fundraiser.currency.symbol + total + '</span>');
        }

        // When the amount changes, change the displayed total.
        $('select[name*="quantity"], input[name*="amount"], input[name*="other_amount"], input[name*="recurs_monthly"]').change(_recalculate_quantity_total());

        // Custom Validation Regex rules: AMEX, VISA, MASTERCARD, DISCOVER, Diner's Club, JCB
        $.validator.addMethod('creditcard', function(value, element) {
          return this.optional(element) || /^(?:4[0-9]{12}(?:[0-9]{3})?|5[1-5]\d{14}$|^2(?:2(?:2[1-9]|[3-9]\d)|[3-6]\d\d|7(?:[01]\d|20))\d{12}|6(?:011|5[0-9][0-9])[0-9]{12}|3[47][0-9]{13}|3[47][0-9]{13}|3(?:0[0-5]|[68][0-9])[0-9]{11}|(?:2131|1800|35\d{3})\d{11})$/i.test(value);
          // Doesn't work for Australian Bankcard, Dankort (PBS) cards or
          // Switch/Solo (Paymentech).
          // Bankcard regexp below needs fixing:
          //^5610\5[6-9]d{2}\d{4}\d{4}$
        }, "Enter a valid credit card number");

        // Custom amount validation
        $.validator.addMethod('amount', function(value, element) {
          // Add regexp
          return this.optional(element) || /^[0-9]*(\.\d{1,3})*(,\d{1,3})?$/i.test(value);
        }, "Enter a valid amount");

        // Custom zipcode validation
        $.validator.addMethod('zipcode', function(value, element) {
          // Validate zip code when country is US.
          var country_field = $(':input[name*="[country]"]');
          if (country_field.length && country_field.val() == 'US') {
            return this.optional(element) || /(^\d{5}((-|\s)\d{4})?$)|(^[ABCEGHJKLMNPRSTVXY]{1}\d{1}[A-Z]{1} *\d{1}[A-Z]{1}\d{1}$)/i.test(value);
          }
          return true;
        }, "Enter a valid zipcode");

        // Instantiate Form Validation
        Drupal.settings.fundraiser.donationValidate = $('.fundraiser-donation-form').validate({
          // Custom keyup function checking for tab key (9) and when value is empty
          onkeyup: function (element, event) {
            if ($(element).next('.error').length){
              if (event.which === 9 && element.value === "") {
                return;
              } else {
                this.element(element);
              }
            }
          },
          onfocusout: function (element) {
            // Callback for real-time onfocusout of form elements.
            var isValid = $(element).valid();

            if (typeof validateKeyCallback == 'undefined') {
              return;
            }

            if (isValid == 0) {
              // Set status to 0.
              validateKeyCallback.status = 0;
              validateKeyCallback.error(element);
            }
            else if (isValid == 1) {
              // Set status to 1.
              validateKeyCallback.status = 1;
              validateKeyCallback.success(element);
            }
          },
          highlight: function(element) {
            $(element).addClass('key-validate');
            $(element).closest('.control-group').removeClass('success').addClass('error');
          },
          success: function(element) {
            $(element).text('OK').addClass('valid').closest('.control-group').removeClass('error').addClass('success');
          }
        });

        // On change and keyup check form status
        $(".fundraiser-donation-form :input.key-validate").bind('change keyup', function() {
          Drupal.settings.fundraiser.donationValidate.element('#' + $(this).attr('id'));
        });

        // Track isValid status of each Braintree hosted field, if we are using that payment method.
        if ($('.braintree-hosted-field').length) {
          var braintreeFields = {'number' : false, 'expirationMonth' : false , 'expirationYear' : false , 'cvv' : false };
          $(document).on('braintree.fieldEvent', function(event, param) {
            var field = param.fields[param.emittedBy];
            var $field = $(field.container);

            if (param.emittedBy == 'cvv') {
              braintreeFields[param.emittedBy] = field.isValid || !field.isEmpty;
            }
            else {
              braintreeFields[param.emittedBy] = field.isValid;
            }

            if (!braintreeFields[param.emittedBy]) {
              $field.closest('.control-group').removeClass('success').addClass('error');
            }
            else {
              $field.closest('.control-group').removeClass('error').addClass('success');
            }
          });
        }

        var formIsValid = function() {
          // If we are using Braintree, both the braintree form and the drupal
          // fields must validate.
          var standardFormValid = Drupal.settings.fundraiser.donationValidate.form();
          if (!standardFormValid) {
            return false;
          }
          else if (undefined !== Drupal.braintreeInstance) {
            if (Drupal.settings.braintree.currentPaymentMethod == 'paypal' || Drupal.settings.braintree.currentPaymentMethod == 'applepay') {
              var $nonce = $('input[name=payment_method_nonce]');
              var $checkedAmounts = $('input[type="radio"][name$="[amount]"]:checked');
              var $otherAmount = $checkedAmounts.filter('[value=other]:visible');
              var $otherAmountValue = $('input[name="submitted[donation][other_amount]"]');
              return $nonce.length > 0 && $checkedAmounts.length && (!$otherAmount.length || $otherAmount.length && $otherAmountValue.val().length);
            }
            else {
              var isBillingUpdate = undefined !== Drupal.settings.braintree.billing_update_type;
              var braintreeFieldsAreValid = function() {
                // Considered valid when not on a billing update form and all
                // fields are filled out, or on a billing update form and either
                // no fields are filled out or all fields are filled out.
                var returnValue = true;
                var atLeastOneTrue = false;
                $.each(braintreeFields, function(index, value) {
                  if (!value) {
                    returnValue = false;
                    // We can return early if not on a billing update form.
                    if (!isBillingUpdate) {
                      return false;
                    }
                  }
                  else {
                    atLeastOneTrue = true;
                  }
                });

                if (!returnValue && isBillingUpdate && !atLeastOneTrue) {
                  returnValue = true;
                }

                return returnValue;
              };

              return braintreeFieldsAreValid();
            }
          }

          return true;
        };

        var $submit = $('.fundraiser-donation-form #edit-submit');
        var $submitMessage = $('.fundraiser_submit_message');

        var $span = $('<span/>').addClass('donation-processing-spinner');
        var $p = $('<p/>').addClass('donation-processing').text('Processing ').append($span);
        var $div = $('<div/>').addClass('donation-processing-wrapper').append($p);
        $div.hide();

        // Add the processing button now since the background needs to be loaded
        // and doing it on submit might cause the background not to load in
        // time.
        $submit.after($div);

        // On submission hide the button and replace it with a new value.
        // Wrap the click in a once trigger to be sure that we bind it only
        // once.
        $('.fundraiser-donation-form').once(function() {
          $('.fundraiser-donation-form').on('submit.donationValidate', function() {
            // Validate the form
            if (formIsValid()) {
              $submit.add($submitMessage).hide();
              $div.show();

              // Scroll to donate button if it's not in view.
              var docTop = $(window).scrollTop();
              var docBottom = docTop + $(window).height();
              var divTop = $div.offset().top;
              var divBottom = divTop + $div.height();
              // Offset in pixels, so that element we scroll to isn't on edge.
              var offset = 50;
              if (divBottom > docBottom || divTop < docTop) {
                var newTop = parseInt(divTop);
                if (divBottom > docBottom) {
                  newTop += offset;
                }
                else {
                  newTop -= offset;
                }
                $('html, body').animate({
                  scrollTop: newTop
                }, 100);
              }

              return true;
            }
            return false;
          });
        });

        // Iterate validation settings and apply rules.
        var $selector;
        if (Drupal.settings.fundraiser && Drupal.settings.fundraiser.js_validation_settings) {
          for ($key in Drupal.settings.fundraiser.js_validation_settings) {
            $selector = $('input[name*="' + $key + '"]');
            if ($selector.length) {
              $selector.rules('add', Drupal.settings.fundraiser.js_validation_settings[$key]);
            }
          }
        }

        // Other Amount
        var $other_amount = $('input[name*="other_amount"][type!="hidden"]');
        $($other_amount).blur(function () {
          var otherAmountField = $(this);
          if (otherAmountField.parent().children('.field-suffix').length > 0) {
            var errorMessage = otherAmountField.parent().children('label.error').detach();
            otherAmountField.parent().children('.description').after(errorMessage);
          }
        });
        if ($other_amount.length) {
          $($other_amount).each(function() {
            var otherAmountField = $(this);
            $(this).rules('add', {
              required: function(element) {
                return $('input[type="radio"][name$="[amount]"]:checked').length == 0 || $('input[type="radio"][name$="[amount]"][value="other"]:visible').is(":checked");
              },
              amount: true,
              min: parseFloat(Drupal.settings.fundraiserWebform.minimum_donation_amount),
              max: parseFloat(Drupal.settings.fundraiserWebform.fundraiser_maximum_other_amount),
              messages: {
                required: "This field is required",
                amount: "Enter a valid amount",
                min: "The amount entered is less than the minimum donation amount."
              }
            });
          });
        }

        // Focus and Blur conditional functions for non-recurring other amount.
        $('input[type="radio"][name*="[amount]"]').change(function(){
          if ($(this).val() == 'other') {
            $('input[name*="[other_amount]"]').focus();
          }
          else {
            clearElement($('input[name*="[other_amount]"]'));
          }
        });

        $('input[name*="[other_amount]"]').focus(function(){
          $('input[type="radio"][name*="[amount]"][value="other"]').attr('checked', 'checked');
        });

        // Focus and Blur conditional functions for recurring other amount.
        $('input[type="radio"][name*="[recurring_amount]"]').change(function(){
          if ($(this).val() == 'other') {
            $('input[name*="[recurring_other_amount]"]').focus();
          }
          else {
            clearElement($('input[name*="[recurring_other_amount]"]'));
          }
        });

        $('input[name*="[recurring_other_amount]"]').focus(function(){
          $('input[type="radio"][name*="[recurring_amount]"][value="other"]').attr('checked', 'checked');
        });

        // Runs on Other Amount field
        $('input[name*="other_amount"]').blur(function(){
          var value = this.value;
          // check for custom validation function object
          if (undefined == window.customValidation) {
            // If the value has length and includes at least one integer
            if (value.length && this.value.match(/\d/g)) {
              // if no period period
              if (!value.match(/\./)) {
                // no decimals: strip all other chars, add decimal and 00
                value = value.replace(/[^\d]+/g,'') + '.00';
              } else {
                // Remove all non-integer/period chars
                value = value.replace(/[^\d\.]+/g,'')
                // make first decimal unique
                  .replace(/\./i,'-')
                  // replace subsequent decimals
                  .replace(/\./g,'')
                  // set first back to normal
                  .replace('-','.')
                  // match the last two digits, removing others
                  .match(/\d+\.\d{0,2}|\.\d{0,2}/);
                var newValue = value[0];
                if (newValue.match(/\.\d{2}/)) {
                } else if (newValue.match(/\.\d{1}/)) {
                  value += '0';
                } else {
                  value += '00';
                }
              }
              this.value = value;
              // total should be recalculated as value has changed without triggering .change() event handler.
              _recalculate_quantity_total();
              $(this).valid();
            }
          } else {
            window.customValidation(value);
            $(this).valid();
          }
        });

        // Ability to override the default message
        jQuery.extend(jQuery.validator.messages, {
          required: "This field is required",
          remote: "Please fix this field",
          email: "Enter a valid email address",
          url: "Enter a valid URL",
          date: "Enter a valid date",
          dateISO: "Enter a valid date (ISO)",
          number: "Must be a number", // changed
          digits: "Enter only digits",
          creditcard: "Enter a valid credit card number",
          equalTo: "Enter the same value again",
          accept: "Enter a value with a valid extension",
          maxlength: jQuery.validator.format("Enter no more than {0} characters"),
          minlength: jQuery.validator.format("Enter at least {0} characters"),
          rangelength: jQuery.validator.format("Enter a value between {0} and {1} characters long"),
          range: jQuery.validator.format("Enter a value between {0} and {1}"),
          max: jQuery.validator.format("Enter a value less than or equal to {0}"),
          min: jQuery.validator.format("Enter a value greater than or equal to {0}")
        });

        // Small helper item
        $('select').each(function(){
          if ($(this).next().is('select')) {
            $(this).next().addClass('spacer');
          }
        });

        // Implementing our own alert close
        // Bootstrap.js uses the .on method, not added until jQuery 1.7
        $('.close').click(function(){
          $(this).closest('.alert').fadeOut();
        });

      }); // window.ready
    } // attach.function
  } // drupal.behaviors
})(jQuery);
