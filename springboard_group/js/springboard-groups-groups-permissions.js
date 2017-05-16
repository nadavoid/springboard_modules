(function ($) {
  Drupal.behaviors.SpringboardGroupHideForm = {
    attach: function (context, settings) {
      // Enables dynamic hiding of the content type permissions on the group permissions page.
      if ($.cookie('Drupal.groupPermissions.showHidden') == 0 || $.cookie('Drupal.groupPermissions.showHidden') === null) {
        $('table#permissions').before('<a href="#" class="show-content-type show-hidden-fields-off">Show Content type permissions</a>');
      }
      else {
        $('table#permissions').before('<a href="#" class="show-content-type show-hidden-fields-on">Hide Content type permissions</a>');
      }
      $('.show-content-type').css('float','right');
      $('.show-content-type').click(function() {
        if ($(this).hasClass('show-hidden-fields-on')) {
          $(this).removeClass('show-hidden-fields-on').addClass('show-hidden-fields-off').text('Show content type permssions');
          $.cookie('Drupal.groupPermissions.showHidden', 0, {
            path: Drupal.settings.basePath,
            // The cookie expires in one year.
            expires: 365
          });
        }
        else {
          $(this).removeClass('show-hidden-fields-off').addClass('show-hidden-fields-on').text('Hide Content type permissions');
          $.cookie('Drupal.groupPermissions.showHidden', 1, {
            path: Drupal.settings.basePath,
            // The cookie expires in one year.
            expires: 365
          });
        }

        $('table#permissions tr').each(function(){
          var classes = $(this).children('td:first').attr('class');
          if ($(this).hasClass('hidden-component-off')) {
            $(this).removeClass('hidden-component-off').addClass('hidden-component-on');
            $(this).children('td:first:contains("Create")').parents('tr').show();
            $(this).children('td:first:contains("Edit own")').parents('tr').show();
            $(this).children('td:first:contains("Edit any")').parents('tr').show();
            $(this).children('td:first:contains("Delete")').parents('tr').show();
          }
          else if ((typeof(classes) != "undefined" &&  classes.indexOf('module-og') !== -1)) {
            $(this).removeClass('hidden-component-on').addClass('hidden-component-off');
            $(this).children('td:first:contains("Create")').parents('tr').hide();
            $(this).children('td:first:contains("Edit own")').parents('tr').hide();
            $(this).children('td:first:contains("Edit any")').parents('tr').hide();
            $(this).children('td:first:contains("Delete")').parents('tr').hide();
          }
        });
        return false;
      });


      var rows = $('table#permissions tr');
      var hidem = false;
      rows.each(function () {
        var id = $(this).children('td:first').attr('id');
        if(Drupal.settings.sbaShowNonMember == 0) {
          $(this).children('th:nth-child(2)').hide();
          $(this).children('td:nth-child(2)').hide();
        }
        var classes = $(this).children('td:first').attr('class');
        if (hidem === true) {
          $(this).hide();
        }
        else {
          if ((typeof(classes) != "undefined" &&  classes.indexOf('module-og') !== -1) && ($.cookie('Drupal.groupPermissions.showHidden') == 0 || $.cookie('Drupal.groupPermissions.showHidden') === null)) {
                $(this).children('td:first:contains("Create")').parents('tr').hide().addClass('hidden-component-off');
                $(this).children('td:first:contains("Edit own")').parents('tr').hide().addClass('hidden-component-off');
                $(this).children('td:first:contains("Edit any")').parents('tr').hide().addClass('hidden-component-off');
                $(this).children('td:first:contains("Delete any")').parents('tr').hide().addClass('hidden-component-off');
                $(this).children('td:first:contains("Delete own")').parents('tr').hide().addClass('hidden-component-off');

          }
        }
      });
    }
  };
})(jQuery);

