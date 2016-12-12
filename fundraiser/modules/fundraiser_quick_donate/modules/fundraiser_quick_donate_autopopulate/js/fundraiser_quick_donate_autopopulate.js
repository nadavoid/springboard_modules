(function($) {
  Drupal.behaviors.fundraiser_quick_donate = {
    attach: function(context, settings) {
      var rand = function(n) {
        return Math.floor(Math.random() * n);
      };
      var $fields = {
        'donation_radios': $('input[name="submitted[donation][amount]"]'),
        'donation_other': $('#edit-submitted-donation-other-amount'),
        'first_name': $('#edit-submitted-donor-information-first-name'),
        'last_name': $('#edit-submitted-donor-information-last-name'),
        'email': $('#edit-submitted-donor-information-mail'),
        'address': $('#edit-submitted-billing-information-address'),
        'city': $('#edit-submitted-billing-information-city'),
        'state': $('#edit-submitted-billing-information-state'),
        'zip': $('#edit-submitted-billing-information-zip'),
        'credit_card': $('#edit-submitted-payment-information-payment-fields-credit-card-number'),
        'cvv': $('#edit-submitted-payment-information-payment-fields-credit-card-cvv')
      };
      var amounts = [];
      $fields['donation_radios'].each(function(i, el) {
        amounts.push($(el).attr('value'));
      });
      var first_names = [
        'Bob',
        'Jeremy',
        'Linda',
        'Carol',
        'Josh',
        'Scott',
        'Hillary',
        'Jennifer'
      ];
      var last_names = [
        'Stephenson',
        'Johnson',
        'McDonnel',
        'Kurtis',
        'Williams'
      ];
      var emails = [
        'tester',
        'randysavage',
        'cartman',
        'icweiner'
      ];
      var addresses = [
        '1776 Washington St',
        '834 Ingersoll Rd',
        '9234 Fairway Ct',
        '3208 MLK Dr'
      ];
      var cities = [
        'Washington D.C.',
        'Des Moines',
        'Los Angeles',
        'Houston',
        'Minneapolis'
      ];
      var states = [
        'IA',
        'NC',
        'MN',
        'TX',
        'CA',
        'NM'
      ];
      var zips = [
        '50320',
        '12345',
        '87321',
        '57329'
      ];
      var credit_cards = [
        '4111111111111111',
        '4222222222222',
        '4012888888881881',
        '5105105105105100'
      ];
      var cvvs = [
        '111',
        '222',
        '321',
      ];

      var $button = $('#fundraiser_quick_donate_autopopulate');
      $button.click(function() {
        var values = {
          'amount': amounts[rand(amounts.length)],
          'first_name': first_names[rand(first_names.length)],
          'last_name': last_names[rand(last_names.length)],
          'email': emails[rand(emails.length)] + '@' + window.location.hostname,
          'address': addresses[rand(addresses.length)],
          'city': cities[rand(cities.length)],
          'state': states[rand(states.length)],
          'zip': zips[rand(zips.length)],
          'credit_card': credit_cards[rand(credit_cards.length)],
          'cvv': cvvs[rand(cvvs.length)]
        };
        var amount_radio = $fields['donation_radios'].filter('[value="' + values.amount + '"]');
        amount_radio.attr('checked', 'checked');
        if (values.amount == 'other') {
          $fields['donation_other'].val(rand(1000));
        }
        else {
          $fields['donation_other'].val('');
        }
        $fields['first_name'].val(values.first_name);
        $fields['last_name'].val(values.last_name);
        $fields['email'].val(values.email);
        $fields['address'].val(values.address);
        $fields['city'].val(values.city);
        $fields['state'].find('option[value="' + values.state + '"]').attr('selected', 'selected');
        $fields['zip'].val(values.zip);
        $fields['credit_card'].val(values.credit_card);
        $fields['cvv'].val(values.cvv);
      });
    }
  }
})(jQuery);
