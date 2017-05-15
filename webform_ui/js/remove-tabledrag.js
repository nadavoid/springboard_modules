(function ($) {
    $(document).ready(function(){
        $('.draggable').each(function(){
            if ($('td:nth-child(2):contains("Hidden")', this).length > 0) {
                console.log($.cookie('Drupal.webformUI.showHidden'));
                if ($.cookie('Drupal.webformUI.showHidden') == 0 || $.cookie('Drupal.webformUI.showHidden') === null) {
                    //$(this).removeClass('draggable tabledrag-leaf')
                    //$(this).find('a.tabledrag-handle').remove();
                    if (!$(this).hasClass('webform-add-form')) {
                        $(this).addClass('hidden-component hidden-component-off').hide();
                    }
                }
                else {
                    if (!$(this).hasClass('webform-add-form')) {
                        $(this).addClass('hidden-component hidden-component-on').show();
                    }
                }

            }
        });

        var addForm = $('tr.webform-add-form');
        $('#webform-components tbody').append(addForm);
        if ($.cookie('Drupal.webformUI.showHidden') == 0 || $.cookie('Drupal.webformUI.showHidden') === null) {
            $('#webform-components-form').prepend('<a href="#" class = "show-hidden-fields show-hidden-fields-off" style="float:left">Show hidden fields</a>');
        }
        else {
            $('#webform-components-form').prepend('<a href="#" class = "show-hidden-fields show-hidden-fields-on" style="float:left">Hide hidden fields</a>');
        }

        $('.show-hidden-fields').click(function() {
            if ($(this).hasClass('show-hidden-fields-off')) {
                $(this).removeClass('show-hidden-fields-off').addClass('show-hidden-fields-on').text('Hide hidden fields');
                $.cookie('Drupal.webformUI.showHidden', 1, {
                    path: Drupal.settings.basePath,
                    // The cookie expires in one year.
                    expires: 365
                });
            }
            else {
                $(this).removeClass('show-hidden-fields-on').addClass('show-hidden-fields-off').text('Show hidden fields');
                $.cookie('Drupal.webformUI.showHidden', 0, {
                    path: Drupal.settings.basePath,
                    // The cookie expires in one year.
                    expires: 365
                });
            }

            $('.hidden-component').each(function(){
                if ($(this).hasClass('hidden-component-off')) {
                    $(this).removeClass('hidden-component-off').addClass('hidden-component-on').show(300);
                }
                else {
                    $(this).removeClass('hidden-component-on').addClass('hidden-component-off').hide(300);
                }

            });
        });
    });
})(jQuery);