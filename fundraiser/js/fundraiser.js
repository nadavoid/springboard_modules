Drupal.behaviors.fundraiserBehavior = {
  attach: function(context) { (function($) {

  // Auto-populate the internal name field
  $("#edit-title").blur(function() {
    var title = $(this).attr('value');
    var currently = $("#edit-internal-name").attr('value');
    if (currently.length === 0) {
      $("#edit-internal-name").val(title);
    }
  });

  // Auto-populate label fields when amount is entered.
  $('#ask-amounts input[id$="amount"]').blur(function() {
    var amount = $(this).val();
    var label = $(this).parents('tr').find('input[id*="label"]').val();
    if ((label.length === 0) && (amount.length !== 0)) {
      $(this).parents('tr').find('input[id*="label"]').val("$" + amount);
    }
  });

  })(jQuery); }
}
