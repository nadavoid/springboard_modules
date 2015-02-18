/**
 * @file
 */

/**
 * Advocacy recipients list  and search page
 */
(function ($) {

    Drupal.behaviors.AdvocacyMessageRecipients = {
        attach: function(context, settings) {
            addButtonState();

            // Apply click event to the search form add links
            // Allows views search results to be appended to the recipients list
            links = $('a.advocacy-add-target, a#advo-add-all');
            links.each(function(){
                $(this, context).once('advocacy-add-target', function() {
                    $(this).click(function (e){
                        e.preventDefault();
                        addTargets('search', this);
                    });
                });
            });

            $('#views-exposed-form-targets-block-3 input, #views-exposed-form-targets-block-3 select').on('change', function(){
                addButtonState();
            })

            var combine, oldVal;
            $('#edit-combine').on('keyup', function() {
                clearTimeout(combine);
                var newVal = $(this).val();
                if (oldVal != newVal) {
                    combine = setTimeout(function() {
                        oldVal = newVal;
                        addButtonState();
                    }, 400);
                }
            })
            $('#quick-target', context).once('advocacy-add-quick-target', function() {
                $('input#quick-target').click(function() {
                    addQuickTarget();
                });
            });

        }
    };

    function addButtonState() {
        var hasValue = false;
        var empty = true;
        $('#views-exposed-form-targets-block-3 input, #views-exposed-form-targets-block-3 select').each(function(){
           if(this.name.indexOf('combine') != -1 || this.name.indexOf('gender') != -1 ||
               this.name.indexOf('social')  != -1 || this.name.indexOf('district') != -1) {
               if ($(this).prop('checked') || (this.name.indexOf('district') != -1 && $(this).val() != 'All')) {
                   hasValue = true;
               }
              if( this.name.indexOf('combine') != -1) {
                  if ($(this).val().length > 0) {
                      hasValue = true;
                  }
              }
           } else {
               if ((this.type=='checkbox' && $(this).prop('checked')) || (this.name == 'search_state' && $(this).val() != 'All')) {
                   empty = false;
               }
           }
        });

        if(hasValue == true || empty == true) {
            $('input#quick-target').prop("disabled", true).fadeTo(400, 0.6).css({'cursor': 'default'}).addClass('cancel-hover');
        }
        else if(empty == false && hasValue == false) {
            $('input#quick-target').prop("disabled", false).fadeTo(200, 1).css({'cursor': 'pointer'}).removeClass('cancel-hover');
        }
    }

    function addQuickTarget() {
        var item = [];
        $('#views-exposed-form-targets-block-3 input, #views-exposed-form-targets-block-3 select').each(function(){
            if ($(this).prop('checked') || (this.name == 'search_state')) {
                name = this.name
                if (name.indexOf('role') != -1) {
                    name = 'role'
                }
                if (name.indexOf('party') != -1) {
                    name = 'party'
                }
                if (name.indexOf('state') != -1) {
                    name = 'state'
                }
                if (this.value != "All") {
                    item.push(name + '=' + this.value);
                }
            }
        });
        console.log(item);
        addTargets('quick', item);
    }

    function addTargets(type, item) {
        var count;

        if ($('.target-recipient').length !== 0) {
            count = parseInt($('.target-recipient').last().attr('id').replace('target-', '')) + 1;
        }
        else {
            count = 0;
        }
        if (type == 'search') {
            var query = $(item).attr('href').replace('add-all?', '').replace('add-target?', '').split('&');
            console.log(query);
        }
        else {
           query = item;
        }
        var readable = buildReadableQuery(query);
        buildDivs(count, readable, query);
        window.topper = $('#springboard-advocacy-message-recipients').height();
        buildUpdateMessage();
        buildFormValue();
    }

    function buildError(messages) {
        err = $('#advo-error-wrapper');
        err.text('');
        err.hide();
        err.css('margin-bottom', 0);
        err.parent().css({'float': 'left'});
        $.each(messages, function(i, message) {
            err.append('<div>' + message + ' field is required </div>');
        });
        err.fadeIn(500);
        err.fadeOut(5000);
    }


    // Rebuild the recipients list on existing messages
    function buildEditPage(recipients) {
       // $('#springboard-advocacy-message-recipients-content').text('');
        var count = 0;
        $.each(JSON.parse(recipients), function(idx, obj) {
            var query = '';
            query = JSON.stringify(obj)
                .replace(/"/g, '')
                .replace(/,/g, '&')
                .replace(/:/g, '=')
                .replace('{', '')
                .replace('}', '')
                .split('&');

            var readable = buildReadableQuery(query);

            buildDivs(count, readable, query);
            count++;
        });
    }

    // Create the recipients list divs, apply data attributes which
    // will be aggregated as a JSON string used by a hidden form field
    // for submission to the API
    function buildDivs(count, readable, query) {
        $('#springboard-advocacy-message-recipients-content')
            .prepend('<div id = "target-' + count + '" class = "target-recipient" style="display: none;">' + readable +
            ' <span><a class ="target-delete" href="#">delete</a></span></div>');
        $('#target-' + count).show(300);
        $('#target-' + count + ' a').click(function(ev){
            ev.preventDefault();
            $(this).closest('.target-recipient').hide(300, function(){
                $(this).remove();
                buildUpdateMessage();
                buildFormValue();
                window.topper = $('#springboard-advocacy-message-recipients').height();
            });

        })
        $(query).each(function(index, value) {
            value = value.split('=');
            $('#target-' + count).attr('data-' + value[0], value[1].replace(/%7C/g, '|'));
        });
    }

    // attaches JSONified data atrributes of the recipients list to a hidden form field
    function buildFormValue() {
        var arr = {};
        $('.target-recipient').each(function(i) {
            arr[i] = $(this).data();
        });
        recipients = JSON.stringify(arr).replace(/"/g, '&quot;');
        $('input[name="data[recipients]"]').val(recipients);
    }

    function buildUpdateMessage() {
        $('.sba-message-status').text('You have unsaved changes').show('slow');
    }

    // Takes a url query string from the search form "add" links
    // and builds readable text for the recipients list.
    function buildReadableQuery(query) {
        var queryObj = {};
        $(query).each(function(index, value) {
            var segments = value.split('=');
            if (segments[0] == 'ids') {
                return false;
            }
            segments[0] = segments[0].ucfirst();
            segments[1] = segments[1].replace(/%7C/g, '|');
            queryObj[segments[0]] = segments[1].split('|');
            $(queryObj[segments[0]]).each(function(i, v){
                switch(v) {
                    case 'FR':
                        queryObj[segments[0]][i] = 'Federal Representatives';
                        break;
                    case 'SR':
                        queryObj[segments[0]][i] = 'State Representatives';
                        break;
                    case 'FS':
                        queryObj[segments[0]][i] = 'Federal Senators';
                        break;
                    case 'SS':
                        queryObj[segments[0]][i] = 'State Senators';
                        break;
                    case 'SX':
                        queryObj[segments[0]][i] = 'State Executives';
                        break;
                    case 'FX':
                        queryObj[segments[0]][i] = 'Federal Executive';
                        break;
                    case 'R':
                        queryObj[segments[0]][i] = 'Republicans';
                        break;
                    case 'D':
                        queryObj[segments[0]][i] = 'Democrats';
                        break;
                    case 'O':
                        queryObj[segments[0]][i] = 'Other';
                        break;
                    case 'M':
                        queryObj[segments[0]][i] = 'Male';
                        break;
                    case 'F':
                        queryObj[segments[0]][i] = 'Female';
                        break;
                }
            });
        });

        if (typeof(queryObj.Id) !== 'undefined') {
            return "Individual: " + queryObj.Sal + " " +  queryObj.First + " " + queryObj.Last;
        }

        cleanUp = JSON.stringify(queryObj).jsonToReadable();
        if (typeof(queryObj.Fields) !== 'undefined' || typeof(queryObj.Gender) !== 'undefined'
            || typeof(queryObj.Social) !== 'undefined' ||  typeof(queryObj.District) !== 'undefined') {
            return "Multiple Individuals: " +  cleanUp;
        }
        return "Group: " +  cleanUp;
    }

    String.prototype.ucfirst = function()
    {
        return this.charAt(0).toUpperCase() + this.substr(1);
    }

    String.prototype.jsonToReadable = function()
    {
      return this.replace(/{"/g, '<ul><li>')
            .replace(/]}/g, '</li></ul>')
            .replace(/\["/g, '')
            .replace(/\],"/g, '</li><li>')
            .replace(/:/g, ': ')
            .replace(/","/g, ', ')
            .replace(/"/g, '')
            .replace(/%20/g, ' ')
    }

    $(document).ready(function () {
        $("#edit-submit, #edit-delete").click(function (e) {
            e.preventDefault();
            var submit = true;
            var messages = [];
            if($('[name*="field_subject_editable"]').length != 0 && !$('[name*="field_subject_editable"]').is(':checked')) {
                messages.push( 'Subject is editable');
                submit = false;
            }
            if($('[name*="field_message_editable"]').length != 0 && !$('[name*="field_message_editable"]').is(':checked')) {
                messages.push('Message is editable');
                submit = false;
            }
            $('input.required').each(function() {
                if ($(this).val() == '') {
                    messages.push($("label[for='" + this.id + "']").text().replace('*', ''));
                    submit = false;
                }
            });
            if(submit == true) {
                $("#sba-message-edit-form").submit();
            }
            else {
                buildError(messages);
            }
        });

        finder = $('.springboard-advocacy-find-targets-container');
        actions = $('#sba-message-edit-form #edit-actions');
        err = $('#advo-error-wrapper');
        err.css({'display': 'inline-block', 'padding-left': '20px'});
        $('#springboard-advocacy-message-form-container').append(finder);
        $(finder).append(actions);
        $(actions).append(err);
        $('input#quick-target').prop("disabled", true).fadeTo(400, 0.6).css({'cursor': 'default'}).addClass('cancel-hover');

        // Editing a pre-existing message, append the recipients
        // to the recipients div using hidden form value
        var recipients =  $('input[name="data[recipients]"]').val();
        if(recipients.length > 0) {
            $('body').once('edit-page', function() {
                $(document).ready(function($){
                    buildEditPage(recipients);
                });
            });
        }
        else {
            $('.sba-message-status').text('No recipients have been selected.').show('slow');
        }

        var offset = $('#springboard-advocacy-message-recipients').offset();

        $(window).scroll(function() {
            // On scroll, update the 'top' value for the summary box to match the
            // scroll distance. If the summary box is absolutely positioned, this
            // will make it folow the scroll down the page.
            if(offset.top <= $(window).scrollTop() && $('#springboard-advocacy-message-recipients').css('position') == 'absolute') {
                newTop =$(window).scrollTop() - offset.top;
                $('#springboard-advocacy-message-recipients').css('top', newTop).addClass('summary-fixed');

            }
            // Clear the positioning if the Summary field is not absolutely positioned.
            // This allows themes to override the fixed position easily.
            else {
                $('#springboard-advocacy-message-recipients').css('top', 0).removeClass('summary-fixed');
            }
        });
    });
})(jQuery);