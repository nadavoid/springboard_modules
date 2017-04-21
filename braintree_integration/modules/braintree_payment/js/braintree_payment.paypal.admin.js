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
  Drupal.behaviors.statesModification = {
    weight: -10,
    attach: function(context, settings) {
      if (Drupal.states) {
        Drupal.states.Dependent.comparisons.Object =
          function(reference, value) {
            if ('regex' in reference) {
              return (new RegExp(reference.regex, reference.flags)).test(value);
            } else {
              return reference.indexOf(value) !== false;
            }
          }
      }
    }
  }
}(jQuery));
