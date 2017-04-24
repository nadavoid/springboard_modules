(function ($) {
  Drupal.behaviors.brainTreePayPalCompact = {
    attach: function (context, settings) {
      var errors = Drupal.settings.paypalCompactErrors || [];
      //if (typeof(Drupal.settings.paypalCompactErrors) != 'undefined') {
        for (i = 0; i < errors.length; i++) {
          $('#webform-components td.tabledrag-hide input.webform-cid').filter(
             function () {
               return this.value === errors[i].cid;
             }
          ).siblings('.webform-pid').val(errors[i].pid)
        }
      //}
    }
  };
  Drupal.behaviors.paypalCompactStates = {
    attach: function(context, settings) {
      this.compactStates();
      $('select[name="gateways[paypal][id]"]').change(this.compactStates);
      $(':input[name="gateways[paypal][status]"]').change(this.compactStates);
    },
    compactStates: function () {
      var payPalOption = $(':input[name="gateways[paypal][status]"]');
      var payPalEnabled = false;
      if (payPalOption.length) {
        payPalEnabled = payPalOption[0].checked;
      }
      var payPalSelect = $('select[name="gateways[paypal][id]"] option');
      var braintreePaypalSelected = false;
      if (payPalSelect.length) {
        $(payPalSelect).each( function() {
          if ($(this).attr('selected')) {
            if (/^braintree_paypal\|/.test($(this).val())) {
              braintreePaypalSelected = true;
            }
          }
        });
      }
      if (payPalEnabled && braintreePaypalSelected) {
        $(':input[name="braintree_paypal_compact"]').parent().show();
      }
      else {
        $(':input[name="braintree_paypal_compact"]').parent().hide();
      }
    }
  };
}(jQuery));
