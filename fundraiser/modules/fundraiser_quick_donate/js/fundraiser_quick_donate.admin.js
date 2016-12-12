(function($) {
  Drupal.behaviors.fundraiser_quick_donate_admin = {
    attach: function(context, settings) {
      $uid = $('#edit-uid');
      $card = $('#edit-card-id');
      var updateCardIds = function() {
        $card.prev('div.card-ids-for-user').remove();
        $.getJSON('/springboard/options/fundraiser/fundraiser_quick_donate/card_ids_by_uid/' + $uid.val(), function(data) {
          $available_cards = $('<div/>', {
            class: 'card-ids-for-user'
          }).html('Available cards for user ' + $uid.val() + ': ' + data.join(', '))
          $card.before($available_cards);
        })
      };
      var timeout;
      $uid.keyup(function() {
        clearTimeout(timeout);
        timeout = setTimeout(function() {
          updateCardIds();
        }, 500);
      })
    }
  }
})(jQuery);
