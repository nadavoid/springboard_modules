(function ($) {
    Drupal.behaviors.fgfGroups = {
        attach: function (context, settings) {
            $('fieldset.ffg-group-funds').once('fund-groups-once', function() {
                $('div.item-list').each(function () {
                    columnize($(this).find('ul'));
                    var list = $(this);
                    list.find('ul').hide();
                    list.find('h3').addClass('group-toggle');
                    var op = list.prepend('<div class="group-toggle">+</div>');
                    list.find('.group-toggle').on('click', function () {
                        list.find('ul').slideToggle('slow', function () {
                            var text = $(this).closest('div.item-list').find('div.group-toggle').text();
                            if (text.indexOf('+') !== -1) {
                                text = text.replace('+', '-')
                                $(this).closest('div.item-list').find('div.group-toggle').text(text)
                            }
                            else {
                                text = text.replace('-', '+')
                                $(this).closest('div.item-list').find('div.group-toggle').text(text)
                            }
                        });
                    });
                });

                $('input.select-all').each(function () {
                    var siblings = $(this).closest('div.item-list').find('.form-checkbox:not(".select-all")');
                    var checkedSiblings = $(this).closest('div.item-list').find('.form-checkbox[checked="checked"]');
                    if (siblings.length == checkedSiblings.length) {
                        $(this).prop("checked", true);
                    }
                });

                $('div.form-item').has('input.select-all').children('label').on('click', function () {
                    checkAll(this);
                });
                $('input.select-all').on('click', function () {
                    checkAll(this);
                });

            });
        }
    };

    function columnize(origList){
        var size = origList.find("li").size();
        var limit = Math.ceil(size / 3);
        for (var i=0; i<3; i++){
            var items = origList.find("li").slice(0, limit);
            if (items.length > 0) {
                var column = $('<ul/>').append(items);
                origList.closest('div.item-list').append(column);
            }
        }
        origList.remove();
    }

    function checkAll(item){
        if (typeof($(item)[0].type) === 'undefined') {
            var checked = true
        }
        else {
            checked = false
        }
        if ($(item).closest('div.item-list').find('input.select-all').prop("checked")) {
            $(item).closest('div.item-list').find('input').prop("checked", !checked);
        }
        else {
            $(item).closest('div.item-list').find('input').prop("checked", checked);
        }
    }

})(jQuery);
