Drupal.behaviors.fundraiserBehavior = {
  attach: function(context, settings) { (function($) {

  // Auto-populate the internal name field
  $('#edit-title').blur(function() {
    var title = $(this).val();
    var internalNameField = $('.form-item input[name*="field_fundraiser_internal_name"]');
    var currently = internalNameField.val();
    if (typeof currently != 'undefined' && currently.length === 0) {
      internalNameField.val(title);
    }
  });

  // Auto-populate label fields when amount is entered.
  $('div[id$="ask-amounts"] input[id$="amount"]').blur(function() {
    var amount = $(this).val();
    var label = $(this).parents('tr').find('input[id*="label"]').val();
    if ((label.length === 0) && (amount.length !== 0)) {
      $(this).parents('tr').find('input[id*="label"]').val("$" + amount);
    }
  });

  // Prevent multiple defaults per set of ask amounts.
  $('#edit-fundraiser-settings input[type=checkbox][name*="[default_amount]"]').change(function(e) {
    if (this.checked) {
      // Uncheck all others.
      $(this).parents('.form-wrapper').first().find(
        'input[type=checkbox][name*="[default_amount]"]:not([name="' + this.name + '"]):checked'
      ).attr('checked', false);
    }
  });

  })(jQuery); }
}
