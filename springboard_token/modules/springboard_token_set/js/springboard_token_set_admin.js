(function ($) {
  Drupal.behaviors.SpringboardTokenFormDebugTool = {
    attach: function (context, settings) {
      if (Drupal.settings['tokenui_debug_form_id'] != undefined) {
        $('input, textarea').each(function () {
          $(this).click(function () {
            if (!$(this).hasClass('tokenui_debug_complete')) {
              $(this).after('<div class="tokenui-debug-tool">form id: <b>' +
                Drupal.settings['tokenui_debug_form_id'] + '</b><br />field name: <b>' + $(this).attr('name') + '</b></div>');
              $(this).addClass('tokenui_debug_complete');
            }
          });
        });
      }
    }
  }; // End SpringboardTokenFormDebugTool
  Drupal.behaviors.SpringboardTokenSetFieldsAdmin = {
    attach: function (context, settings) {
      var submitButton = $('#springboard-token-set-fields-admin-form #edit-token-add');
      var formSelect = $('#springboard-token-set-fields-admin-form #edit-target-form-id');
      var customFormSelect = $('#springboard-token-set-fields-admin-form .form-item-custom-target-form-id input');
      var fieldSelect = $('#springboard-token-set-fields-admin-form #edit-field-key');
      var customFieldSelect = $('#springboard-token-set-fields-admin-form .form-item-custom-field-key input');
      var tokenSetSelect = $('#springboard-token-set-fields-admin-form #edit-token-set');
      customFieldSelect.after('<div><a href="#" class="hide-custom-field-select">(show fields)</a></div>');
      customFieldSelect.parent().find('.hide-custom-field-select').each(function () {
        $(this).click(function (e) {
          e.preventDefault();
          fieldSelect.parent().show();
          fieldSelect.parent().removeClass('form-disabled');
          customFieldSelect.val('');
          customFieldSelect.parent().hide();
          customFieldSelect.parent().removeClass('form-disabled');
          fieldSelect.prop('selectedIndex', 0);
          tokenSetSelect.removeAttr('disabled');
          tokenSetSelect.parent().removeClass('form-disabled');
          updateSelectedTokenSets();
        });
      });
      fieldSelect.attr('disabled', 'disabled');
      fieldSelect.parent().addClass('form-disabled');
      tokenSetSelect.attr('disabled', 'disabled');
      tokenSetSelect.parent().addClass('form-disabled');
      submitButton.hide();
      customFormSelect.parent().hide();
      customFieldSelect.parent().hide();
      $('#springboard-token-set-fields-admin-form #edit-target-form-id').change(function () {
        if ($(this).val() == 0) {
          fieldSelect.attr('disabled', 'disabled');
          fieldSelect.parent().addClass('form-disabled');
          fieldSelect.empty();
          tokenSetSelect.attr('disabled', 'disabled');
          tokenSetSelect.parent().addClass('form-disabled');
          fieldSelect.append('<option value="0">- First, please select a target form. -</option>' + "\n");
          submitButton.hide();
          return;
        }
        if ($(this).val() == '___custom') {
          tokenSetSelect.children().each(function () {
            $(this).removeAttr('selected');
          });
          customFormSelect.parent().show();
          customFormSelect.val('');
          customFormSelect.focus();
          customFieldSelect.val('');
          customFieldSelect.parent().find('.hide-custom-field-select').parent().hide();
          customFieldSelect.parent().show();
          customFieldSelect.parent().addClass('form-disabled');
          customFieldSelect.attr('disabled', 'disabled');
          fieldSelect.parent().hide();
          tokenSetSelect.parent().addClass('form-disabled');
          submitButton.hide();
        }
        else {
          customFormSelect.val('');
          customFormSelect.blur();
          customFormSelect.parent().hide();
          customFieldSelect.parent().find('.hide-custom-field-select').parent().show();
          customFieldSelect.parent().hide();
          customFieldSelect.parent().removeClass('form-disabled');
          customFieldSelect.removeAttr('disabled');
          fieldSelect.parent().show();
          $.ajax({
            url: '/sb-admin-token-fields-ajax/' + $(this).val(),
            dataType: 'json',
            context: document.body,
            success: function(data) {
              fieldSelect.empty();
              fieldSelect.removeAttr('disabled');
              fieldSelect.parent().removeClass('form-disabled');
              tokenSetSelect.removeAttr('disabled');
              tokenSetSelect.parent().removeClass('form-disabled');
              for (var field_name in data) {
                fieldSelect.append('<option enabled_tsids="' + data[field_name] + '" value="' + field_name  + '">' + field_name + '</option>' + "\n");
              }
              updateSelectedTokenSets();
              fieldSelect.append('<option value="__custom_field">- New Field -</option>');
            }
          });
        }
      });
      fieldSelect.change(function () {
        updateSelectedTokenSets();
      });
      var updateSelectedTokenSets = function () {
        tokenSetSelect.children().each(function () {
          $(this).removeAttr('selected');
        });
        var selectedOption = fieldSelect.children().filter(":selected");
        if (selectedOption.val() == '__custom_field') {
          fieldSelect.parent().hide();
          customFieldSelect.val('');
          customFieldSelect.parent().show();
          customFieldSelect.focus();
          submitButton.hide();
          tokenSetSelect.attr('disabled', 'disabled');
          tokenSetSelect.parent().addClass('form-disabled');
        }
        else {
          var enabledTokenSets = selectedOption.attr('enabled_tsids');
          enabledTokenSets = enabledTokenSets.split(',');
          var tokenSetsWereSelected = true;
          for (var i = 0; i < enabledTokenSets.length; i++) {
            tokenSetSelect.children().each(function () {
              if ($(this).attr('value') == enabledTokenSets[i]) {
                $(this).attr('selected', 'selected');
              }
            });
          }
          if (tokenSetsWereSelected == true) {
            submitButton.show();
          }
        }
      };
      customFormSelect.keyup(function () {
        if ($(this).val() == '') {
          customFieldSelect.parent().addClass('form-disabled');
          customFieldSelect.attr('disabled', 'disabled');
        }
        else {
          customFieldSelect.parent().removeClass('form-disabled');
          customFieldSelect.removeAttr('disabled');
        }
      });
      customFieldSelect.keyup(function () {
        if ($(this).val() == '') {
          tokenSetSelect.attr('disabled', 'disabled');
          tokenSetSelect.parent().addClass('form-disabled');
          tokenSetSelect.val('');
          submitButton.hide();
        }
        else {
          tokenSetSelect.removeAttr('disabled');
          tokenSetSelect.parent().removeClass('form-disabled');
          if (fieldSelect.val() != '' && tokenSetSelect.val() > 0) {
            submitButton.show();
          }          
        }
      });
      tokenSetSelect.change(function () {
        if (tokenSetSelect.val() == 0 || !tokenSetSelect.val()) {
          submitButton.hide();
        }
        else {
          submitButton.show();
        }
      });
      $('#springboard-token-set-fields-admin-form').submit(function (e) {
        if (formSelect.val() == '' || fieldSelect.val() == '' || tokenSetSelect.val() == '0' || !tokenSetSelect.val()) {
          e.preventDefault();
        }
      });
      $('#sb-token-fields-list .sb-tsid-remove').click(function (e) {
        var parentRow = $(this).parent().parent();
        e.preventDefault();
        $.post(
          '/sb-admin-token-fields-remove-ajax', {
            'tsid' : $(this).attr('tsid'),
            'field_name' : $(this).attr('field_name'),
            'form_id' : $(this).attr('form_id')
          },
          function(data) {
            if (data != 'success') {
              alert(data);
            }
            else {
              parentRow.remove(); 
            }
          }
        );
      });
    }
  }; // End SpringboardTokenSetFieldsAdmin

  Drupal.behaviors.SpringboardTokenSetAdmin = {
    attach: function (context, settings) {
      $('#springboard-token-set-sets-admin-form .sb-token-descr').change(function () {
        $(this).parent().parent().children('.update-token-link').show();
      });
      $('#springboard-token-set-sets-admin-form .sb-token-descr').keydown(function () {
        $(this).parent().parent().children('.update-token-link').show();
      });
      $('#springboard-token-set-sets-admin-form .update-token-link').click(function (e) {
        e.preventDefault();
        _sbTokenSetManagerUpdateToken($(this));
      });
      function _sbTokenSetManagerUpdateToken(updateLink) {
        $.post(
          '/sb-admin-token-update-ajax', {
            'tid' : updateLink.attr('tid'),
            'descr' : updateLink.parent().find('.sb-token-descr').val(),
          },
          function(data) {
            if (data != 'success') {
              alert(data);
            }
            else {
              updateLink.hide();
            }
          }
        );
      } 
      function _sbTokenSetManagerRemoveToken(delLink) {
        $.post(
          '/sb-admin-token-delete-ajax', {
            'tsid' : delLink.attr('tsid'),
            'tid' : delLink.attr('tid')
          },
          function(data) {
            if (data != 'success') {
              alert(data);
            }
            else {
              delLink.parent().remove();
            }
          }
        );
      }
      $('#springboard-token-set-sets-admin-form .form-type-textfield').keydown(function(e) {
        if(e.keyCode == 13) {
          e.preventDefault();
          return false;
        }
      });
      $('#springboard-token-set-sets-admin-form .remove-token-link').click(function (e) {
        e.preventDefault();
        _sbTokenSetManagerRemoveToken($(this));
      });
      $('#springboard-token-set-sets-admin-form .add-token-button').click(function (e) {
        var submitButton = $(this);
        e.preventDefault();
        $.post(
          '/sb-admin-token-add-ajax', {
            'tsid' : $(this).attr('tsid'),
            'token_machine' : $(this).parent().find('.token-machine').val(),
            'token_descr' : $(this).parent().find('.token-description').val()
          },
          function(data) {
            if (!$.isNumeric(data)) {
              alert(data);
            }
            else {
              var tsid = submitButton.attr('tsid');
              var tid = data;
              var tokenMachine = submitButton.parent().find('.token-machine').val();
              submitButton.parent().before(
                '<div class="form-wrapper">' +
                  '<div class="form-item form-type-textfield form-disabled">' +
                    '<input type="text" ' +
                      'disabled="disabled" name="token-sets[token-set-' + tsid + '][' + tokenMachine + '][token]" ' +
                      'value="' + tokenMachine + '" size="60" maxlength="128" class="form-text">' +
                  '</div>' + "\n" +
                  '<div class="form-item form-type-textfield form-disabled">' +
                    '<input type="text" ' +
                      'disabled="disabled" name="token-sets[token-set-' + tsid + '][' + tokenMachine + '][token_description]" ' +
                      'value="' + submitButton.parent().find('.token-description').val() + '" size="60" maxlength="128" class="form-text">' +
                  '</div>' + "\n" +
                  '<a href="#" class="update-token-link" style="display:none;" id="update-token-' + tsid + '-' + tid + '" tsid="' + tsid + '" tid="' + tid + '">Update token</a>' +
                  '<a href="#" class="remove-token-link" id="remove-token-' + tsid + '-' + tid + '" tsid="' + tsid + '" tid="' + tid + '">Remove token</a>' +
                '</div>'
              );
              $('#remove-token-' + tsid + '-' + tid).click(function (e) {
                e.preventDefault();
                _sbTokenSetManagerRemoveToken($(this));
              });
              submitButton.parent().find('.token-machine').val('');
              submitButton.parent().find('.token-description').val('');
            }
          }
        );
      });
    }
  }; // End SpringboardTokenSetAdmin
})(jQuery);
