(function($) {
  Drupal.behaviors.fundraiser_ocd_fundraiser = {
    attach: function(context, settings) {
      if (Drupal.settings.fundraiser.ocd.readonly) {
        var fieldsets = {};
        var $readonly = {};
        var readonlyize = function(elements) {
          $.each(elements, function(key, data) {
            fieldsets[key] = [];
            $readonly[key] = $('<fieldset/>', {class: 'readonly-information'});
            $readonly[key].append($('<legend/>').text(data.title));
            var children = data.children;
            $.each(children, function(parentId, value) {
              var fullId = 'webform-component-' + parentId;
              fieldsets[key].push('#' + fullId);
              var $fieldset = $('#' + fullId);
              var $div = $('<div/>');
              $.each(value, function(index, group) {
                var addBreak = false;

                $.each(group, function(childIndex, childId) {
                  if (childId.indexOf('value:') < 0) {
                    var $child = $('#edit-submitted-' + parentId + '-' + childId);
                    if ($child.children('.form-type-radio').length) {
                      var $value = $child.find('input:checked').next('label').text();
                      $div.append($('<span/>', {class: childId}).text($value));
                    }
                    else if ($child.val().length) {
                      var $value = $child.val();
                      if (childIndex > 0) {
                        $value = ' ' + $value;
                      }
                      $div.append($('<span/>', {class: childId}).text($value));
                      addBreak = true;
                    }
                  }
                  else {
                    childId = childId.replace('value:', '');
                    $div.append(childId);
                  }
                });
                if (addBreak) {
                  $div.append('<br/>');
                }
              });

              $readonly[key].append($div);
              $fieldset.hide();
            });

            $readonly[key].insertAfter(data.after);

            var hidden = true;
            var $editlink = $('<a/>', {href: '#'}).text('edit').click(function() {
              if (hidden) {
                $(fieldsets[key].join(',')).show();
                $readonly[key].hide();
                hidden = false;
              }
              else {
                $(fieldsets[key].join(',')).hide();
                $readonly[key].show();
                hidden = true;
              }
              return false;
            });
            $readonly[key].append($editlink);
          });
        };

        readonlyize({
          'your-information': {
            'title': Drupal.t('Your Information'),
            'children': {
              'donor-information': [['first-name', 'last-name']],
              'billing-information': [
                ['address'],
                ['address-line-2'],
                ['city', 'value:,', 'state', 'zip']
              ]
            },
            'after': '#webform-component-donation'
          },
          'payment-information': {
            'title': Drupal.t('Payment Information'),
            'children': {
              'payment-information': [['payment-fields-credit-cardonfile']]
            },
            'after': '#webform-component-billing-information'
          }
        });
      }
    }
  };
})(jQuery);
