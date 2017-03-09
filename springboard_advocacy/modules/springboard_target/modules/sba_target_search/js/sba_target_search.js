/**
 * @file
 */

/**
 * Advocacy recipients list  and search page
 */
(function ($) {

    // Functions which need to fire on initial page load AND ajax reloads.
    Drupal.behaviors.AdvocacyMessageRecipients = {
        attach: function(context, settings) {
            //manipulate form elements based on subscription level
            Sba.buildSubscriptions();

            // define trigger for custom event handler used by State dropdown's ajax callback.
            // Reset district option elements to default on State element change.
            Sba.buildStateField();

            // Set up tabs and UI wrapping elements for target search.
            // Renamed from buildCommitteeSearch();
            Sba.buildSearchUI();

            // Define click events for add target links
            Sba.buildTargetLinkEvent(context);

            // Remove jQuery Uniform from selectors
            Sba.deUniform();

            //Update exposed form element states when a district is selected
            Sba.districtReloader(context);

            // Insert placeholder text and update exposed form elements when combo search is changed.
            Sba.comboSearchUpdater();

            //No results message div
            if($('.view-targets div.view-empty:visible').length != 0) {
                var actions = $('#edit-actions', 'form.sba_target_search_parent_form'); // removed additional selector #sba-message-edit-form, doesn't seem to be necessary
                $('div.view-empty').clone().appendTo(actions);
                $('.view-targets div.view-empty').remove();
                $('.view-empty', '#edit-actions').show();
            }

            // Prevent doubleclicks of target search submit.
            var subButton = $('#edit-submit-targets');
            subButton.ajaxStart(function() {
                subButton.prop('disabled', true).css({'cursor': 'not-allowed'});
            });
            subButton.ajaxComplete(function()  {
                subButton.prop('disabled', false).css({'cursor': 'pointer'});
            });
            Sba.scroller(context);
        }
    };


    // Functions which need to happen on initial page load, but not ajax reload
    $(document).ready(function () {

        // Hide the no search results message
        $('.view-empty').hide();

        //stop views ajax auto scroll from moving the exposed search form to top of screen
        $('.pager-item a, #edit-submit-targets').click(function(){
            Drupal.ajax.prototype.commands.viewsScrollTop = null;
        });

        // Add the show/hide toggle for admin-facing user-editable textarea
        Sba.userEditableFormDisplay();

        // Define click events for submit and delete buttons
        Sba.messageFormSubmitter();

        // rearrange/append recipients container, error message div, submit button
        Sba.prepareMessageForm();

        Sba.setFormValue();

        //Sba.scroller();
    });


    /*********  Function Definitions **************/

    //namespace
    window.Sba = {};

    //manipulate form elements based on subscription level
    Sba.buildSubscriptions = function () {

        if (typeof(Drupal.settings.sbaSubscriptionLevel) !== "undefined") {
            var sub = Drupal.settings.sbaSubscriptionLevel;
            //Federal Only
            if (sub == 'federal-only') {
                $("option", "#edit-search-committee-chamber").each(function () {
                    if ($(this).html().indexOf('State') != -1) {
                        $(this).remove();
                    }
                });
                if (window.search_state == 'committee') {
                    $('#edit-search-state-wrapper').hide();
                }
                else {
                    $('#edit-search-state-wrapper').show();
                }
            }
            //State only
            if (sub == 'state-only') {
                $("option", "#edit-search-committee-chamber").each(function () {
                    if ($(this).html().indexOf('Federal') != -1) {
                        $(this).remove();
                    }
                });
            }

            //Selected states only
            if (sub == 'states-selected') {
                $("option", "#edit-search-committee-chamber ").each(function () {
                    if ($(this).html().indexOf('Federal') != -1) {
                        $(this).remove();
                    }
                });

            }

            //Federal and some states
            if (sub == 'federal-and-states-selected') {
                if(window.search_state == 'committee') {
                    $("option", "#edit-search-state").each(function () {
                        if ($(this).val() != "All" && $.inArray($(this).val(), Drupal.settings.sbaAllowedStates) == -1) {
                            $(this).hide();
                        }
                    });
                }
                else {
                    $("option", "#edit-search-state").each(function () {
                        $(this).show();
                    });
                }
            }

            // Initialize some setup functions for federal-and-states-selected subscription level
            // to hide/show various options
            if (sub == 'federal-and-states-selected') {
                //main search
                Sba.toggleStateAndBranchState();
                Sba.toggleStateAndBranchBranch();
                //committee search
                Sba.toggleStateAndChambersState();
                Sba.toggleStateAndChambersChamber();
            }
        }
    };

    // define trigger for custom event handler used by State dropdown's ajax callback.
    // Reset district option elements to default on State element change.
    Sba.buildStateField = function () {
        var state = $('#edit-search-state');
        var searchSub = $('#edit-submit-targets');
        state.ajaxComplete(function() {
            searchSub.prop('disabled', false).css({'cursor': 'pointer'});
        });

        // if only some states are allowed, or federal and and some states,
        // only trigger the district field update if certain criteria are met
        if(typeof(Drupal.settings.sbaAllowedStates) !== "undefined") {
            state.on('change', function () {
                if($(this).val() != 'All' && ($.inArray($(this).val(), Drupal.settings.sbaAllowedStates) !=-1
                    || Drupal.settings.sbaSubscriptionLevel == 'federal-and-states-selected')) {
                    if(window.search_state != 'committee') {
                        $(this).trigger('custom_event');
                        searchSub.prop('disabled', true).css({'cursor': 'not-allowed'});
                    }
                }
                else {
                    $('select[name="search_district_name"]').html('<option selected="selected" value="All">- Any -</option>');
                }
            });
        }
        // Else all states are allowed
        else {
            state.on('change', function () {
                $('select[name="search_district_name"]').html('<option selected="selected" value="All">- Any -</option>');
                if($(this).val() != 'All') {
                    if(window.search_state != 'committee') {
                        searchSub.prop('disabled', true).css({'cursor': 'not-allowed'});
                        $(this).trigger('custom_event');

                    }
                }
            });
        }
    };

    // Set up tabs and other shared UI elements for target search.
    // renamed from buildCommitteeSearch().
    Sba.buildSearchUI = function () {
        // Add faux tabs
        $('.view-targets').once('advocacy-search-tabs', function () {
            var finder = $('.view-targets', '#springboard-advocacy-find-targets-container');
            // create faux sub-tabs for custom targets
            finder.prepend('<div class="faux-subtab-container"><div class="faux-tab custom-individual"><a href ="#custom-individual" class="custom-search custom-individual-subsearch">Individuals</a></div>');
            var isSocial = $('div[id*="edit-field-sba-twitter-message"]');
            if (isSocial.length === 0) {
                $('.faux-subtab-container').append('<div class="faux-tab custom-groups"><a href ="#custom-groups" class="custom-groups-subsearch">Groups</a></div></div>');
            }
            // create faux primary tabs
            finder.prepend('<div class="faux-tab-container"><div class="faux-tab legislative"' +
                '><a href ="#full" class="legislative-search">Legislative Targets</a></div><' +
                'div class="faux-tab committee"><a href ="#committee" class="committee-search">Legislative Committees</a></div><' +
                'div class="faux-tab custom"><a href ="#custom" class="custom-search">Custom Targets</a></div>');
            // Hide the 'type' switch
            $("#edit-search-class-name-wrapper").hide();
        });

        // Legislative target specific search elements
        var legWidgets = $('#edit-search-role-1-wrapper, #edit-search-party-wrapper, #edit-search-social-wrapper, #edit-search-gender-wrapper, #edit-search-district-name-wrapper, #edit-combine-wrapper');
        // Legislative committee specific search elements
        var comWidgets = $('#edit-search-committee-chamber-wrapper, #edit-search-committee-wrapper');
        // Custom target specific search elements
        var custWidgets = $('.faux-subtab-container'); // both custom options
        var custIndividualWidgets  = $('#edit-combine-wrapper'); // custom individual search only
        var custGroupWidgets = $('#edit-group-name-wrapper');  // custom group search only
        // Elements to hide only on custom tab
        var custHideWidgets = $('#state-district-wrapper');

        //committee tab click event
        $('.committee-search').click(function () {
            $('.faux-tab').removeClass('active');
            Sba.reset('committee');
            Sba.setTargetType("Legislator");
            window.search_state = 'committee';
            legWidgets.hide();
            custWidgets.hide();
            custGroupWidgets.hide();
            custIndividualWidgets.hide();
            custHideWidgets.show();
            $('.views-targets-button-wrappers, .search-reset').hide(); //breaks if put in hideWidget object
            comWidgets.show(300);
            Sba.buildSubscriptions();
            Sba.toggleStateAndChambers();
            Sba.CommitteeElStates(true);
            $('.faux-tab.committee').addClass('active');
            return false;
        });

        //custom tab click event
        $('.custom-search').click(function () {
            $('div.narrow-search').remove();
            $('.faux-tab').removeClass('active');
            window.search_state = 'custom';
            Sba.reset('custom');
            Sba.setTargetType("Target");
            legWidgets.hide();
            custGroupWidgets.hide();
            comWidgets.hide();
            custHideWidgets.hide();
            $('.views-targets-button-wrappers, .search-reset').hide(); //breaks if put in hideWidget object
            custWidgets.show(300);
            custIndividualWidgets.show(300);
            Sba.buildSubscriptions();
            // Sba.toggleStateAndChambers();
            // Sba.CommitteeElStates(false);
          $('#edit-submit-targets').attr('value', 'Search').show();
          $('.faux-tab.custom, .faux-tab.custom-individual').addClass('active');
          return false;
        });

        //custom subtab click event
        $('.custom-groups-subsearch').click(function () {
            $('div.narrow-search').remove();
            $('.faux-tab').removeClass('active');
            window.search_state = 'custom-group';
            Sba.reset('custom');
            Sba.setTargetType("Group");
            legWidgets.hide();
            custIndividualWidgets.hide();
            comWidgets.hide();
            custHideWidgets.hide();
            $('.views-targets-button-wrappers, .search-reset').hide(); //breaks if put in hideWidget object
            custWidgets.show(300);
            custGroupWidgets.show(300);
            Sba.buildSubscriptions();
            //Sba.toggleStateAndChambers();
            //Sba.CommitteeElStates(false);
          $('#edit-submit-targets').attr('value', 'Search').show();
          $('.faux-tab.custom, .faux-tab.custom-groups').addClass('active');
            return false;
        });

        //legislative target tab click event
        $('.legislative-search').click(function () {
            $('div.narrow-search').remove();
            $('.faux-tab').removeClass('active');
            Sba.setTargetType("Legislator");
            window.search_state = 'legislative';
            comWidgets.hide();
            custWidgets.hide();
            custGroupWidgets.hide();
            custIndividualWidgets.hide();
            custHideWidgets.show();
            legWidgets.show(300);
            $('.search-reset').show(300);
            Sba.buildSubscriptions();
            Sba.reset('committee');
            Sba.reset('custom');
            $('#edit-submit-targets').attr('value', 'Search').show();
            $('#edit-search-state').prop('disabled', false).removeClass('disabled');
            $('.faux-tab.legislative').addClass('active');
            return false;
        });

        // update committee fields after autocomplete textfield update
        var comField = $('#edit-search-committee');
        comField.on('blur', function() { // "change" no work in ie11
            Sba.CommitteeElStates();
        });

        //set up some defaults based on current window state after ajax reload
        if(window.search_state == 'committee' ) {
            $('.faux-tab').removeClass('active');
            Sba.setTargetType("Legislator");
            $('.views-targets-button-wrapper').hide();
            legWidgets.hide();
            custWidgets.hide();
            custGroupWidgets.hide();
            custIndividualWidgets.hide();
            custHideWidgets.show();
            //$('.views-targets-button-wrappers, .search-reset').hide();
            comWidgets.show(300);
            $('.faux-tab.committee').addClass('active');
            Sba.CommitteeElStates();
        }
        else if(window.search_state == 'custom') {
            $('.faux-tab').removeClass('active');
            Sba.setTargetType("Target");
            //Sba.toggleStateAndBranch();
            custGroupWidgets.hide();
            legWidgets.hide();
            comWidgets.hide();
            custHideWidgets.hide();
            custWidgets.show(300);
            custIndividualWidgets.show(300);
            $('.faux-tab.custom-individual').addClass('active');
            $('#edit-submit-targets').show().attr('value', 'Search');
        }
        else if(window.search_state == 'custom-group') {
            $('.faux-tab').removeClass('active');
            Sba.setTargetType("Group");
            //Sba.toggleStateAndBranch();
            custIndividualWidgets.hide();
            legWidgets.hide();
            comWidgets.hide();
            custHideWidgets.hide();
            custWidgets.show(300);
            custGroupWidgets.show(300);
            $('.faux-tab.custom-groups').addClass('active');
            $('#edit-submit-targets').show().attr('value', 'Search');
        }
        else {
            $('.faux-tab').removeClass('active');
            Sba.setTargetType("Legislator");
            Sba.toggleStateAndBranch();
            comWidgets.hide();
            custWidgets.hide();
            custGroupWidgets.hide();
            custIndividualWidgets.hide();
            custHideWidgets.show();
            legWidgets.show(300);
            $('#edit-submit-targets').show().attr('value', 'Search');
            $('.faux-tab.legislative').addClass('active');
        }

      //set up up default placeholder text in legislative/individual search textfield
      var legField = $('#edit-combine');
        var desc = $('#edit-combine-wrapper .description');
        var legPlaceholder = desc.text().trim();
        desc.hide();
        if (legField.val().length == 0) {
            legField.attr('placeholder', legPlaceholder);
        }
        legField.focus(function () {
            $(this).attr('placeholder', '');
        });
        legField.blur(function() {
            if(legField.val().length == 0) {
                $(this).attr('placeholder', legPlaceholder);
            }
        });


        //set up up default placeholder text in group search textfield
        var groupField = $('#edit-group-name');
        var groupDesc = $('#edit-group-name-wrapper .description');
        var groupPlaceholder = groupDesc.text().trim();
        groupDesc.hide();
        if (groupField.val().length == 0) {
            groupField.attr('placeholder', groupPlaceholder);
        }
        groupField.focus(function () {
            $(this).attr('placeholder', '');
        });
        groupField.blur(function() {
            if(groupField.val().length == 0) {
                $(this).attr('placeholder', groupPlaceholder);
            }
        });


        //set up up default placeholder text in name search textfield
        var comDesc = $('.description', '#edit-search-committee-wrapper');
        var comPlaceholder = comDesc.text().trim();
        comDesc.hide();
        if (comField.text().length == 0) {
            comField.attr('placeholder', comPlaceholder);
        }
        comField.focus(function () {
            $(this).attr('placeholder', '');
        });
        comField.blur(function() {
            if(comField.val().length == 0) {
                $(this).attr('placeholder', comPlaceholder);
            }
        });

        // Hide target results if search criteria are updated
        $('input', '#edit-search-committee-wrapper').on('keydown', function(){
            if($('div.view-targets').is(':visible')) {
                Sba.reset();
            }
            Sba.setElStates();
        });
    };

    // Define click events for add target links
    Sba.buildTargetLinkEvent = function (context) {
        // Apply click event to the search form add links
        // Allows views search results to be appended to the recipients list
        var links = $('a.advocacy-add-target, a#advo-add-all');
        links.each(function() {
            $(this, context).once('advocacy-add-target', function() {
                $(this).click(function (e){
                    e.preventDefault();
                    Sba.addTarget('search', this);
                });
            });
        });

        //set the click event for the quick target button
        $('#quick-target', context).once('advocacy-add-quick-target', function() {
            Sba.setElStates();//need this?
            Sba.reset('all');
            $('input#quick-target').click(function() {
                //$('.views-targets-button-wrapper').hide();
                var query = Sba.getQuickQuery();
                var arr = Sba.addTarget('quick', query);
                return false;

            });
        });
    };

    // Remove jQuery Uniform stylings from select elements
    Sba.deUniform = function () {
        if ($.isFunction($.fn.uniform)) {
            $('select').each(function () {
                $.uniform.restore(this);
            });
        }
    };

    // If a district is selected other form elements need to be enabled/disabled/hidden
    Sba.districtReloader = function (context) {
        $('select[name="search_district_name"]', context).once('advocacy-district-reloaded', function() {
            Sba.setElStates();
            $('#views-exposed-form-targets-block-3 input, #views-exposed-form-targets-block-3 select').on('change', function(){
                if(this.type !== 'button' && this.type !== 'hidden') {
                    if($('div.view-targets').is(':visible')) {
                        Sba.reset();
                    }
                    $('.view-empty', '#edit-actions').hide();

                    Sba.setElStates();
                }
            });
        });
    };

    // Update the main search form based on keyword textfield state
    Sba.comboSearchUpdater = function () {
        var combineField = $('#edit-combine');
        //placeholder text
        if (combineField.text().length === 0) {
            var desc = $('.description', '#edit-combine-wrapper');
            var placeholder = desc.text().trim();
            desc.hide();
            combineField.attr('placeholder', placeholder);
            combineField.focus(function () {
                $(this).attr('placeholder', '');
            });
        }

        //update exposed form element states when the text search field is changed
        var combine, oldVal;
        combineField.on('keyup', function() {
            clearTimeout(combine);
            var newVal = $(this).val();
            if (oldVal !== newVal) {
                combine = setTimeout(function() {
                    oldVal = newVal;
                    if($('div.view-targets').is(':visible')) {
                        Sba.reset();
                    }
                    Sba.setElStates();
                }, 400);
            }
        });
    };

    // show/hide user editable textarea, and update the scroller position of the recipients div
    Sba.userEditableFormDisplay = function () {
        var showEdit = $('input[name*=field_sba_user_editable]');
        var editable = $('#sba_message_sba_message_action_message_form_group_sba_editable, #edit-field-sba-bottom-conclusion');
        if(showEdit.prop('checked')) {
            editable.show();
            Sba.scroller();
        }
        showEdit.on('change', function() {
            if(this.checked) {
                editable.show(400, 'linear');
                Sba.scroller();
            }
            else {
                editable.hide(400, 'linear');
                Sba.scroller();
            }
        });
    };

    //set up the event for the message save button validation and submit
    Sba.messageFormSubmitter = function () {
        /* messages specific

         $("#edit-submit, #edit-delete").click(function (e) {
            e.preventDefault();
            var messages = [];
            //if($('[name*="field_sba_subject_editable"]').length != 0 && !$('[name*="field_sba_subject_editable"]').is(':checked')) {
            //    messages.push( 'Subject is editable');
            //}
            //if($('[name*="field_message_editable"]').length != 0 && !$('[name*="field_message_editable"]').is(':checked')) {
            //    messages.push('Message is editable');
            //}
            $('input.required').each(function() {
                if ($(this).val() == '') {
                    messages.push({type: 'required', message: $("label[for='" + this.id + "']").text().replace('*', '')});
                }
            });
            if(typeof(Drupal.settings.charCount) !== 'undefined') {

                var handleCount = Drupal.settings.charCount.size;
                var $text = $('div[id*="edit-field-sba-twitter-message"]').find('textarea');
                $text.each(function () {
                    if (this.id.indexOf('edit-field-sba-twitter-message-und') != -1) {

                        var currLen = $(this).val().replace(/(\r\n|\n|\r)/gm, "").length;
                        if (currLen > 140 - handleCount) {
                            messages.push({type: 'error', message: 'Message is too long for all potential targets.'});
                        }
                    }
                });
                var hasDistrict = $('input[name*=field_sba_target_options]:checked').val();
                if (hasDistrict == 0) {
                    var limit = true;
                    var targetCount = Drupal.settings.charCount.count;
                    //if (targetCount > 4 && targetCount < 26) {
                    //    limit = confirm("Are you sure? Current settings will result in " + targetCount + ' tweets for this message.');
                    //}
                    //if (!limit) {
                    //    return false;
                    //}
                    if (targetCount > 5) {
                        messages.push({
                            type: 'error',
                            message: 'This message will generate more than 5 tweets per user, please revisit your target options and try again.'
                        });
                    }
                }
            }

            if(messages.length === 0) {
                console.log('submit trigger');
                $("#edit-submit-hidden").trigger('click'); // message-specific

            }
            else {
                console.log('error trigger'
                );
                Sba.setError(messages);
            }
        });*/
    };

    //rearrange some elements on the exposed form and recipients container
    Sba.prepareMessageForm = function () {
        var recipContainer = $('#springboard-advocacy-target-recipients-container');
        var finder = $('#springboard-advocacy-find-targets-container');
        var actions = $('#edit-actions', 'form.sba_target_search_parent_form');
        var err = $('#advo-error-wrapper');

        $('form.sba_target_search_parent_form').after(finder);
        finder.append(recipContainer);
        finder.append(actions);
        actions.prepend(err);
        $('.views-targets-button-wrapper').hide();

        // Editing a pre-existing message, append the recipients
        // to the recipients div using hidden form value
        var recipients =  $('input[type="hidden"][name$="data[recipients]"]').val();
        if(recipients.length > 0) {
            $('body').once('edit-page', function() {
                Sba.buildEditPage(recipients);
            });
        }
        else {
            $('.sba-message-status').text('No recipients have been selected.').show('slow');
        }
    };


    //recipients container scroll calculations
    Sba.scroller = function (context) {
        var id = '';
        if (typeof(context) !== 'undefined' && typeof(context[0]) !== 'undefined') {
            id = context[0].id;
        }
        if ((typeof(context) !== 'undefined' && typeof(context[0]) === 'undefined') || id.indexOf('add-more-wrapper') != -1 || id.indexOf('message-edit-form') != -1) {
            setTimeout(function () {
                var recips = $('#springboard-advocacy-target-recipients-container .inner');
                var offset = recips.offset();
                var newTop;
                if (recips.hasClass('recipients-fixed')) {
                    newTop = $(window).scrollTop() - offset.top;
                    recips.css('top', newTop).removeClass('recipients-fixed');
                }
                $(window).scroll(function () {
                    var footerOffset = $('#footer-wrapper').offset();
                    if (offset.top <= $(window).scrollTop() && recips.css('position') == 'absolute') {
                        var recipHeight = recips.height();
                        var newTop = $(window).scrollTop() - offset.top;
                        if ((offset.top + newTop + recipHeight + 20) < footerOffset.top) {
                            recips.css('top', newTop).addClass('recipients-fixed');
                        }
                    }
                    else {
                        recips.css('top', 0).removeClass('recipients-fixed');
                    }
                });

            }, 50);
        }
    };

    // Update committee search form elements
    // for federal-and-states-selected subscription level
    Sba.toggleStateAndChambers = function () {
        //state/chamber event driven enabling goes here.
        if (window.search_state == 'committee') {
            $('select', '#edit-search-committee-chamber-wrapper').change(function() {
                Sba.toggleStateAndChambersChamber();
            });

            $('select', '#edit-search-state-wrapper').change(function() {
                Sba.toggleStateAndChambersState();
            });
        }
    };

    //disable state dropdown if a federal chamber is selected
    Sba.toggleStateAndChambersChamber = function () {
        var chamber = $('select', '#edit-search-committee-chamber-wrapper');
        var text = $( '#' + chamber.id + ' option:selected').text();
        var state = $('#edit-search-state');
        if(text.indexOf('Federal') != -1 ) {
            state.prop('disabled', true);
            state.addClass('disabled');
            state.siblings('label').css({'cursor': 'not-allowed'});
        }
        else {
            state.prop('disabled', false);
            state.siblings('label').css({'cursor': 'default'});
        }
    };

    //disable federal chambers if a state is selected
    Sba.toggleStateAndChambersState = function () {
        var state = $('select', '#edit-search-state-wrapper');
        var chamberOption = $("option", '#edit-search-committee-chamber');
        if (state.val() != 'All') {
           chamberOption.each(function() {
                if($(this).html().indexOf('Federal') != -1) {
                    $(this).hide();
                }
            });
        }
        else {
           chamberOption.each(function() {
                if($(this).html().indexOf('Federal') != -1) {
                    $(this).show();
                }
            });
        }
    };

    // Update target search form elements
    // for federal-and-states-selected subscription level
    Sba.toggleStateAndBranch = function () {
        var sub = Drupal.settings.sbaSubscriptionLevel;
        if (sub == 'federal-and-states-selected') {
            var states = Drupal.settings.sbaAllowedStates;
            $('select', '#edit-search-state-wrapper').change(function () {
                Sba.toggleStateAndBranchState();
            });

            $('input', '#edit-search-role-1-wrapper').change(function () {
                Sba.toggleStateAndBranchBranch();
            });
        }
    };

    // Disable state branches when an unsubscribed state is selected
    Sba.toggleStateAndBranchState = function () {
        var value = $('select', '#edit-search-state-wrapper').val();
        if (value != 'All' && $.inArray(value, Drupal.settings.sbaAllowedStates) == -1) {
            $("input", "#edit-search-role-1-wrapper ").each(function () {
                if ($(this).siblings('label').html().indexOf('Federal') == -1) {
                    $(this).closest('.control-group').hide();
                }
            });
        }
        else {
            $("input", "#edit-search-role-1-wrapper").each(function () {
                $(this).closest('.control-group').show();
            });
        }
    };

    // disable unsubscribed states in the state dropdown when a state branch is checked
    Sba.toggleStateAndBranchBranch = function () {
        var checkedState = false;
        var boxes = $('input', '#edit-search-role-1-wrapper');
        boxes.each(function(){
            var fed = $(this).siblings('label').html().indexOf('State');
            if($(this).prop('checked') &&  fed != -1) {
                checkedState = true;
            }
        });
        var options = $('select option', '#edit-search-state-wrapper');
        options.each(function () {
            if ($.inArray($(this).val(), Drupal.settings.sbaAllowedStates) == -1) {
                if (checkedState == true) {
                    $(this).hide();
                }
                else {
                    $(this).show();
                }
            }
        });
    };

    // selectively disable/enable exposed form elements based on user actions
    Sba.setElStates = function () {

        //element state meta-variables
        var notGroupable = false;
        var groupable = false;
        var hasDistrict = false;
        var hasState = false;
        //elements
        var combine = $('input#edit-combine');
        var district = $('select[name="search_district_name"]');
        var state = $('select[name="search_state"]');
        var allBoxes = $('input[type="checkbox"]', '#views-exposed-form-targets-block-3');
        var allInputs =$('#views-exposed-form-targets-block-3 input[type="checkbox"], #views-exposed-form-targets-block-3 input[type="text"], #views-exposed-form-targets-block-3 select');

        allInputs.each(function() {
            //update our element state meta-variables
            var nm = this.name;
            if(nm.indexOf('combine') != -1 || nm.indexOf('district') != -1 ) {
                if ($(this).prop('checked') || (nm.indexOf('district') != -1 && district.val() != 'All')) {
                    notGroupable = true;
                    if (nm.indexOf('district') != -1) {
                        hasDistrict = true;
                        notGroupable = false;
                    }
                }
                if(nm.indexOf('district') != -1 && district.val() == 'All') {
                    hasDistrict = false;
                }

                if( nm.indexOf('combine') != -1) {
                    if ($(this).val().length > 0) {
                        notGroupable = true;
                    }
                }
            } else {

                if (this.type=='checkbox' && $(this).prop('checked')) {
                    groupable = true;
                }
                if(nm == 'search_state' && $(this).val() != 'All') {
                    hasState = true;
                }
            }
        });

        if ($("input[name*=field_sba_target_option]").length != 0) {
            var districted_tweet = $("input[name*=field_sba_target_option]:checked").val();
            if (districted_tweet != 1) {
                notGroupable = true;
                $('#advo-add-all').hide();
            }
            else {
                $('#advo-add-all').show();
            }
        }

        //update form element states based on meta-variables
        if(hasDistrict == true){
            allBoxes.each(function(){
                $(this).prop('disabled', true);
                $(this).closest('.views-exposed-widget').addClass('disabled');
                $(this).siblings('label').css({'cursor': 'not-allowed'});
            });
            combine.prop('disabled', true);
            combine.closest('.views-exposed-widget').addClass('disabled');

        }
        if(hasDistrict == false) {
            allBoxes.each(function(){
                if($(this).prop('disabled') == true) {
                    $(this).prop('disabled', false);
                    $(this).closest('.views-exposed-widget').removeClass('disabled');
                    $(this).siblings('label').css({'cursor': 'pointer'});

                }
            });
            combine.prop('disabled', false);
            combine.closest('.views-exposed-widget').removeClass('disabled');
            if(state.val() != "All" && groupable == false && notGroupable == false && $('select[name="search_district_name"] option').size() > 1) {
                district.prop('disabled', false);
                district.removeClass('disabled');

                //jquery uniform
                $('div.disabled', '#edit-search-district-name-wrapper').removeClass('disabled');
            }
            else {
                district.prop('disabled', true);
                district.addClass('disabled');
                //jquery uniform
                $('div.selector', '#edit-search-district-name-wrapper').addClass('disabled');
            }
        }
        //update quick target button based on meta-variables
        if( notGroupable == true || (groupable == false && hasState == false)) {
            if(window.search_state != 'committee') {
                $('.views-targets-button-wrapper')
                    .prop("disabled", true)
                    .fadeOut(400, function() {
                        $(this).hide(500);
                    })
                    .css({'cursor': 'default'})
                    .addClass('cancel-hover');
                $('.views-targets-button-wrapper input')
                    .prop("disabled", true)
                    .fadeOut(400,  function() {
                        $(this).hide(500);
                    })
                    .css({'cursor': 'default'})
                    .addClass('cancel-hover');
            }
        }
        else if((groupable == true || hasState == true)  && window.search_state != 'committee' && notGroupable == false) {
            $('.views-targets-button-wrapper').prop("disabled", false).fadeIn(200).css({'cursor': 'pointer'}).removeClass('cancel-hover');
            $('.views-targets-button-wrapper input').prop("disabled", false).fadeIn(200).css({'cursor': 'pointer'}).removeClass('cancel-hover');
        }
    };

    // Update committee search elements based on autocomplete field state
    Sba.CommitteeElStates = function(committeeTab) {

        var state  = $('#edit-search-state');
        var stateWrap =$('#state-district-wrapper');
        var submit = $('#edit-submit-targets');
        var chamber = $('#edit-search-committee-chamber');
        var commit = $('#edit-search-committee');

        stateWrap.append($('#edit-search-committee-chamber-wrapper'));
        $('div.narrow-search').remove();
        stateWrap.before('<div class = "narrow-search">You can narrow the autocomplete results by state or chamber.</div>');

        // Find if there is a committee id in the autocomplete field
        var pattern = /(id:[0-9]+)/;
        var hasId = pattern.test(commit.val());

        // If there's a valid committee, enable the search submit button
        if(hasId === true) {
            submit.show('400');
            $('.search-reset').show();
            submit.attr('value', 'Get Members');
            state.prop('disabled', true);
            state.addClass('disabled');
            chamber.prop('disabled', true);
            chamber.addClass('disabled');
            $('.views-targets-button-wrapper').prop("disabled", false).fadeIn(200).css({'cursor': 'pointer'}).removeClass('cancel-hover');
            $('.views-targets-button-wrapper input').prop("disabled", false).fadeIn(200).css({'cursor': 'pointer'}).removeClass('cancel-hover');
        }
        else {
            submit.hide('400');
            $('.search-reset').hide();
            if (committeeTab != true && commit.val() != '') {
                commit.val('').attr('placeholder', 'Invalid search. Please select a suggested committee name from the list.');
            }
            state.prop('disabled', false);
            state.removeClass('disabled');
            chamber.prop('disabled', false);
            chamber.removeClass('disabled');
            $('.views-targets-button-wrapper')
              .prop("disabled", true)
              .fadeOut(400, function() {
                  $(this).hide(500);
              })
              .css({'cursor': 'default'})
              .addClass('cancel-hover');
            $('.views-targets-button-wrapper input')
              .prop("disabled", true)
              .fadeOut(400,  function() {
                  $(this).hide(500);
              })
              .css({'cursor': 'default'})
              .addClass('cancel-hover');

            }
    };

    // Set the hidden search type field
    Sba.setTargetType = function (type) {
      $('#edit-search-class-name').val(type);
    };

    // quick target click function, builds query array
    Sba.getQuickQuery = function () {

        var roles = [];
        var parties = [];
        var states = [];
        var genders = [];
        var socials = [];
        var committee = [];
        var committee_id = [];
        var districts = [];

        //iterate through the form elements and build arrays of checked/selected items
        $('#views-exposed-form-targets-block-3 input[type="checkbox"], #views-exposed-form-targets-block-3 select, #edit-search-committee').each(function(){

            if ($(this).prop('checked') || this.name == 'search_state' || (this.name == 'search_district_name' && this.value != "All")  || (this.name == 'search_committee' && this.value != '')) {
                var nm = this.name;
                var v = this.value;
                if (nm.indexOf('search_committee') != -1) {
                    var com_id = v.match("id:[0-9]*");
                    committee.push(v.replace(/\(id:[0-9]*\)/g, ''));
                    committee_id.push(com_id[0].replace('id:', ''));
                }
                else if (nm.indexOf('role') != -1) {
                    roles.push(v);
                }
                else if (nm.indexOf('party') != -1) {
                    parties.push(v);
                }
                else if (nm.indexOf('state') != -1) {
                    if (typeof(Drupal.settings.sbaSubscriptionIsGrouped) !== "undefined" && Drupal.settings.sbaSubscriptionIsGrouped && this.value == "All") {
                        allowedStates = Drupal.settings.sbaAllowedStates;
                        $(allowedStates).each(function(){
                            states.push(this);
                          });
                    }
                    if(this.value != "All") {
                        states.push(v);
                    }
                }
                else if (nm.indexOf('district') != -1) {
                    districts.push(v);
                }
                else if (nm.indexOf('gender') != -1) {
                    genders.push(v);
                }
                else if (nm.indexOf('social') != -1) {
                    socials.push(v);
                }
                else return false;
            }
        });

        roles = roles.toString().replace(/,/g, '|');
        parties = parties.toString().replace(/,/g, '|');
        states = states.toString().replace(/,/g, '|');
        genders = genders.toString().replace(/,/g, '|');
        socials = socials.toString().replace(/,/g, '|');
        districts = districts.toString();

        var query = [];
        if(committee.length > 0) {
            query.push('committee=' + committee);
            query.push('committee_id=' + committee_id);
        }
        if(states.length > 0) {
            query.push('state=' + states);
        }
        if(roles.length > 0) {
            query.push('role=' + roles);
        }
        if(parties.length > 0) {
            query.push('party=' + parties);
        }
        if(genders.length > 0) {
            query.push('gender=' + genders);
        }
        if(socials.length > 0) {
            query.push('social=' + socials);
        }
        if(districts.length > 0) {
            query.push('district_name=' + districts);
        }
        return query;
    };

    // add target
    Sba.addTarget = function (type, query) {
        if (type == 'search') {
            var qArr = $(query).attr('href').replace('add-all?', '').replace('add-target?', '').split('&');
        }
        else {
           qArr = query;
        }

        var dupe = Sba.addTargetDedupe(qArr);
        if(dupe == true) {
            return false;
        }

        var readable = Sba.buildReadableQuery(qArr);
        var id = Sba.calcDivId();
        Sba.buildDiv(id, readable, qArr, false);
        Sba.setUpdateMessage('');
        Sba.setCountMessage();
        Sba.setFormValue();
    };

    // create uniqie IDs for each recipient group
    Sba.calcDivId = function (){
        var recips = $('.target-recipient');
        if (recips.length !== 0) {
            var count = recips.map(function() {
                return $(this).attr('id').replace('target-', '');
            }).get().sort(function(a, b){
                return a-b
            });
            var  id = parseInt(count.pop()) + 1;
        }
        else {
            id = 0;
        }
        return id;
    };

    //dedupe add target requests
    Sba.addTargetDedupe = function(newQueryArr) {
        var duplicate;
        var soloTarget;

        // the new and old queries get translated into both arrays and objects
        // in order to facilitate various key/value and string comparisons
        var newQueryObj = {};
        var oldQueryArr = [];
        var oldQueryObj = {};

        //convert the new query array to an object
        $.each(newQueryArr, function(i, v){
            var segments = v.split('=');
            newQueryObj[segments[0]] = segments[1];
        });

        // iterate through the existing targets and build data object and arrays for each.
        // then compare them to the new query
        $('.target-recipient').each(function() {

            oldQueryObj = $(this).data();
            oldQueryArr = $.map(oldQueryObj, function(value, key) {
                return key + '=' + value;
            });

            var newQueryString =  newQueryArr.toString().replace(/%7C/g, '|');
            var oldQueryString = oldQueryArr.toString();
            soloTarget = newQueryString.indexOf('id=');

            // if the new and old queries are identical strings, we're done here
            if (newQueryString == oldQueryString) {
                duplicate = true;
            }

            //If the queries have the same key count, but they are not identical,
            // they could be different keys or the same key with different values
            if(oldQueryArr.length == newQueryArr.length && duplicate != true) {
                var missing;

                $.each(newQueryObj, function (key, values) {

                    //there's a new key here, definitely not a dupe
                    if(!oldQueryObj.hasOwnProperty(key)){
                        missing = true;
                    }

                    //It's the same key, but maybe the values are different.
                    if (missing != true) {
                        var newQueryValuesArr = values.split('|');
                        $.each(newQueryValuesArr, function(i, value){
                            if(oldQueryObj[key].toString().indexOf(value) == -1) {
                                missing = true;
                            }
                            else {
                                duplicate = true;
                            }
                        });
                    }
                });

                if(missing == true) {
                    duplicate = false;
                }
            }

            // if the new query has more items than the old query
            if(oldQueryArr.length < newQueryArr.length && duplicate != true) {
                var missing;

                //it's not a dupe if the overlapping keys have different values
                $.each(newQueryObj, function (key, values) {
                    if(oldQueryObj.hasOwnProperty(key)){
                        var newQueryValuesArr = values.split('|');
                        $.each(newQueryValuesArr, function(i, value){
                            if(oldQueryObj[key].toString().indexOf(value) == -1 || value == '') {
                                missing = true;
                            }
                            else {
                                duplicate = true;
                            }
                        });
                    }
                });

                if(missing == true) {
                    duplicate = false;
                }
            }

            //if the new query has less keys than the old query, it's not a dupe
            if(oldQueryArr.length > newQueryArr.length &&  duplicate != true) {
                duplicate = false;
            }
        });  // end $('target-recipients').each();

        if(duplicate == true) {
            if(soloTarget == -1) {
                var message = 'Duplicate Targets. The group you are trying to add is already fully contained within another group.'
            }
            else {
                message = 'Duplicate Target.'
            }
            Sba.setUpdateMessage(message);
            return true;
        }
        return false;
    };

    // validation error message displayed next to save button
    Sba.setError =function (messages) {
        var err = $('#advo-error-wrapper');
        err.text('').hide().css('margin-bottom', 0);
        err.prepend('<div class = "advo-warning"><strong>Oops!</strong> it looks like you missed the following:</div>');
        $.each(messages, function(i, message) {
          if (message.type == 'required') {
            err.append('<div>' + message.message + ' field is required </div>');
          }
          else {
            err.append('<div>' + message.message + '</div>');
          }
        });
        err.fadeIn(500);//.delay(7000).fadeOut(1000);
    };

    // Rebuild the recipients list on existing messages
    Sba.buildEditPage = function (recipients) {
        $('#springboard-advocacy-target-recipients-content').text('');
        recipients = recipients.replace(/&quot;/g, '"');
        $.each(JSON.parse(recipients), function(id, obj) {
            var query = $.map(obj, function(value, name) {
                if (!obj.hasOwnProperty('search_committee')) {
                    return name + '=' + value
                }
                // Make pre-groups committees into group-era committees
                else if(obj.hasOwnProperty('search_committee') && name != 'ids') {
                    if (!obj.hasOwnProperty('committee_id')) {
                        // The id: colon may or may not be escaped, possibly.
                        var com_id = value.match("id%3A[0-9]*");
                        if (com_id) {
                            com_id = com_id[0].replace('id%3A', '');
                            return ["committee=" + value, "committee_id=" + com_id]
                        }
                        else {
                            com_id = value.match("id:[0-9]*");
                            if (com_id) {
                                com_id = com_id[0].replace('id:', '');
                                return ["committee=" + value, "committee_id=" + com_id]
                            }
                            else {
                                return ["committee=Unknown Committee", "committee_id="]
                            }
                        }
                    }
                    else {
                        return name + '=' + value
                    }
                }
            });
            var readable = Sba.buildReadableQuery(query);
            Sba.buildDiv(id, readable, query, 'reload');
        });
        //reverse the display order so newest are at top.
        var content = $('#springboard-advocacy-target-recipients-content');
        var contentItems = content.children('.target-recipient');
        content.append(contentItems.get().reverse());
        Sba.setCountMessage();
    };

    // Create the recipients list divs, apply data attributes which
    // will be aggregated as a JSON string used by a hidden form field
    // for submission to the API
    Sba.buildDiv = function (id, readable, query, reload) {
        if (reload == false) {
            var isnew = 'new';
        }
        else{
            isnew = 'reloaded';
        }
        $('#springboard-advocacy-target-recipients-content')
            .prepend('<div id = "target-' + id + '" class = "target-recipient ' + isnew +'" style="display: none;">' + readable +
            ' <span><a class ="target-delete remove-target" href="#"></a></span></div>');
        $('#target-' + id).show(200).addClass('trans');
        $('#target-' + id + ' a').click(function(ev){
            ev.preventDefault();
            $(this).closest('.target-recipient').hide(300, function(){
                $(this).remove();
                Sba.setUpdateMessage('');
                Sba.setFormValue();
                Sba.setCountMessage();
            });
        });
        $(query).each(function(i, v) {
            v = v.split('=');
            $('#target-' + id).attr('data-' + v[0], v[1].replace(/%7C/g, '|'));
        });
    };

    // attaches JSONified data attributes of the recipients list to a hidden form field
    Sba.setFormValue = function () {
        var obj = {};
        $('.target-recipient').each(function(i) {
            obj[i] = $(this).data();
        });
        recipients = JSON.stringify(obj).replace(/"/g, '&quot;');
        $('input[name="data[recipients]"]').val(recipients);

        if(typeof(Drupal.settings.charCount) !== 'undefined') {
          Sba.ajaxSearch(obj);
        }
    };

    // Unsaved changes message in recipients box
    Sba.setUpdateMessage = function (message) {
        var status = $('.sba-message-status');
        if(message == '') {
            message = 'You have unsaved changes.'
        }
        else {
            var current =  status.text();
            if(current.indexOf('You have') != -1) {
                message = message + " You have unsaved changes."
            }
        }
        status.hide().text(message).show('slow');
    };

     // Count recipients and groups and display a message.
     Sba.setCountMessage = function () {
        $('.targeting-count, .remove-all-targets').remove();
        var message = [];
        var groups = parseInt($('.target-recipient .title').length);
        var individs = parseInt($('.target-recipient .legislator').length) + parseInt($('.target-recipient .individual').length);
        var gCount = groups > 1 ? 'groups' : 'group';
        var iCount = individs > 1 ? 'individuals' : 'individual';

        if(groups > 0) {
            message.push(groups + ' ' + gCount);
        }
        if(individs > 0 ) {
            message.push(individs + ' ' + iCount)
        }

        var countMessage = '<strong>Targeting: </strong> ' + message.join(' & ');
        var counter = '<div class="targeting-count">' + countMessage + '</div><a class="remove-all-targets">Remove All Targets</a></div>';

         if(individs > 0 || groups > 0) {
            $('#springboard-advocacy-target-recipients-content').append(counter);
        }
         else {
             $('#edit-save-target-group').show().removeClass('hidden');

         }

        $('.remove-all-targets').click(function() {

            $('.target-recipient').remove();
            $('.targeting-count, .remove-all-targets').remove();
            // Reveal 'save as group' option if present.
            $('#edit-save-target-group').show().removeClass('hidden');
            Sba.setUpdateMessage('');
            Sba.setFormValue();
        });
    };

    // Takes a url query string from the search form "add" links
    // or the quick target parameters
    // and builds readable text for the recipients list.
    Sba.buildReadableQuery = function (query) {

      var queryObj = {};
        $(query).each(function(index, value) {
            var segments = value.split('=');
            if (segments[0] == 'ids') {
              return false;
            }
            if (typeof(segments[1]) == "undefined") {
                return true;
            }

            segments[0] = segments[0].SbaUcfirst();
          if(segments[0] != 'Search_committee' && segments[0] != 'Committee_id' && segments[0] != 'Class_name') {
                segments[1] = segments[1].replace(/%7C/g, '|');
              queryObj[segments[0]] = segments[1].split('|');
            }
            // If this is a legislative group...
            else if(segments[0] == 'Search_committee') {
              segments[1] = segments[1].replace(/%7C/g, ' ');
                queryObj['Committee'] = segments[1];
            }

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
                    case 'PRES01':
                        queryObj[segments[0]][i] = 'US President';
                        break;
                    case 'PRES03':
                        queryObj[segments[0]][i] = 'US Vice-President';
                        break;
                    case 'GOVNR':
                        queryObj[segments[0]][i] = 'State Governors';
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
                    case 'twitter':
                        queryObj[segments[0]][i] = 'Twitter';
                        break;
                    case 'facebook':
                        queryObj[segments[0]][i] = 'Facebook';
                        break;
                }
            });
        });

        // Individual targets
        if (typeof(queryObj.Id) !== 'undefined') {
            var type = '';
            var group = '';
            var sal = '';
            var first = '';
            var last = '';
            var party = '';
            var org = '';
            var title = '';
            var state = '';
            var district = '';
            if (typeof(queryObj.Type) !== 'undefined') {
                type = queryObj.Type.toString().SbaStrCln();
            }
            if (typeof(queryObj.Name) !== 'undefined') {
                group = queryObj.Name.toString().SbaStrCln();
            }
            if (typeof(queryObj.Sal) !== 'undefined') {
                sal = queryObj.Sal.toString().SbaStrCln();
            }
            if (typeof(queryObj.First) !== 'undefined') {
                first = queryObj.First.toString().SbaStrCln();
            }
            if (typeof(queryObj.Last) !== 'undefined') {
                last = queryObj.Last.toString().SbaStrCln();
            }
            if (typeof(queryObj.Title) !== 'undefined') {
                title = queryObj.Title.toString().SbaStrCln();
            }
            if (typeof(queryObj.Org) !== 'undefined') {
                org = queryObj.Org.toString().SbaStrCln();
            }
            if (typeof(queryObj.District) !== 'undefined') {
                district = queryObj.District.toString().SbaStrCln();
            }
            if (typeof(queryObj.State) !== 'undefined') {
                state = queryObj.State.toString().SbaStrCln();
            }
            if (typeof(queryObj.Party) !== 'undefined') {
                if (type == 'Legislator') {
                    party = queryObj.Party.toString().SbaStrCln();
                    if (party.length > 0) {
                        party = '(' + party + ')';
                    }
                }
            }

            var separator = title !='' && org != '' ? ', ' : ' ';
            // Custom targets and executive branch.
            var non_legislative_organization = org != ''  ? title + separator + org : '';

            var legislative_organization = district != '' && title != '' ? title + ', ' +  district  :  org;
            var affiliation = district != ''  ? legislative_organization : non_legislative_organization;

            // Add class to distinquish between legislative targets and custom individual or group targets.
            var divClass = 'target legislator';
            var displayTitle = sal + " " +  first + " " + last + ' ' + party  + "<br />" + affiliation;
            if (type == 'group') {
                divClass = 'target group';
                displayTitle = '<div class="title"><h4>' + group + '</h4></div>';
            }
            else if(type == 'Target') {
                divClass = 'target individual';
            }

            return  '<div class="' + divClass + '">' + displayTitle +'</div>';
        }

        // If this is a queried group (rather than an editable group entity)
        //
        var cleanUp = JSON.stringify(queryObj).SbaJsonToReadable();
        if (typeof(queryObj.Fields) !== 'undefined' || typeof(queryObj.Genderxxxx) !== 'undefined'
            || typeof(queryObj.Socialxxxx) !== 'undefined' ||  typeof(queryObj.District) !== 'undefined') {
            return '<div class="title"><h4>Group Target</h4></div>' +  cleanUp;
        }
        return '<div class="title"><h4>Group Target</h4></div> ' +  cleanUp;
    };

    // Reset form element states, hide old search results
   Sba.reset = function (type) {
        if (type == 'all') {
            $('.views-submit-button').append('<a class = "search-reset" href ="#">reset</a>');
            $('.search-reset').click(function () {
                Sba.resetFull();
                return false;
            });
        }
        else if (type == 'committee') {
            Sba.resetFull();
        }
        else if (type == 'custom') {
          Sba.resetFull();
        }
        else {
            $('.view-empty').hide();
            $('.view-content').fadeOut(333);
            $('.attachment').fadeOut(333);
            $('div.view-targets .item-list').fadeOut(333);
            //$('#advocacy-attachment-before-add-all').fadeOut(333);

        }
    };

    // Reset form and page as if it were reloaded.
    Sba.resetFull = function () {
        $('.view-empty').hide();

        var allInputs = $('#views-exposed-form-targets-block-3 input[type="checkbox"], #views-exposed-form-targets-block-3 input[type="text"], #views-exposed-form-targets-block-3 select');
        allInputs.each(function () {
            if ($(this).prop('tagName') == "SELECT" && $(this).prop('id') != 'edit-search-class-name') {
                $(this).prop('selectedIndex', 0);
                if ($.isFunction($.fn.uniform)) {
                    $.uniform.restore(this);
                }
            }
            if (this.type == 'checkbox') {
                $(this).prop('checked', false);
                $(this).prop('disabled', false);
                $(this).removeClass('disabled');
                $(this).siblings('label').removeClass('disabled').css({'cursor': 'pointer'});
                $(this).closest('.views-exposed-widget').removeClass('disabled');
            }
            if (this.type == 'text') {
                $(this).val('');
                $(this).attr('value', ''); // I'm not sure why this is needed, but the POST values were not being cleared with .val()
                $(this).prop('disabled', false);
                $(this).removeClass('disabled');
            }
        });
        $('.views-targets-button-wrapper').fadeOut(333);
        $('.view-content').fadeOut(333);
        $('.attachment').fadeOut(333);
        $('div.view-targets .item-list').fadeOut(333);

        // Don't disable the district field if there's only one state in the subscription level.
        var skip = false;
        if(typeof(Drupal.settings.sbaAllowedStates) !== "undefined" && Drupal.settings.sbaAllowedStates.length == 1) {
            skip = true;
        }
        if (!skip) {
            $('select[name="search_district_name"]').prop('disabled', true).addClass('disabled');
        }

        if(window.search_state == 'committee') {
            Sba.CommitteeElStates(true);
        }

        return false;
    };

    Sba.ajaxSearch = function(arr) {
        if(!$.isEmptyObject(arr)) {
            $.ajax({
                type: "POST",
                url: Drupal.settings.sbaSiteUrl,
                data: {query: arr},
                dataType: 'json',
                success: function (data) {
                    Drupal.settings.charCount = data;
                    sbaCountable.charCount();
                },
                error: function (xhr, textStatus, error) {
                }
            });
        }
        else {
            Drupal.settings.charCount = {size: 0, count: 0, person: ""}
            sbaCountable.charCount();
        }
    };

    //Uc first helper
    String.prototype.SbaUcfirst = function()
    {
        return this.charAt(0).toUpperCase() + this.substr(1);
    };

    //json notation to readable string helper
    String.prototype.SbaJsonToReadable = function()
    {
      return this.replace(/\{"/g, '<ul><li class ="heading">')
            .replace(/\]\}/g, '</li></ul>')
            .replace(/\["/g, '<li>')
            .replace(/\],"/g, '</li></ul><ul><li class ="heading">')
            .replace(/:/g, '</li>')
            .replace(/","/g, '<li>')
            .replace(/"/g, '')
            .replace(/%20/g, ' ')
            .replace(/%2C/g, ', ')
            .replace(/%28/g, '(')
            .replace(/%29/g, ')')
            .replace(/%3A/g, ':')
            .replace(/%2B/g, '+')
            .replace('District_name', 'District')
            .replace(/\}/g, '');
    };

    String.prototype.SbaStrCln = function()
    {
        var word = this;
        return word.replace(/"/g, '')
          .replace(/%20/g, ' ')
          .replace(/%2C/g, ', ')
          .replace(/%28/g, '(')
          .replace(/%27/g, '\'')
          .replace(/%26/g, '&')
          .replace(/%29/g, ')')
          .replace(/%3A/g, ':')
          .replace(/%2B/g, '+')
          .replace('Republicans', 'R')
          .replace('Democrats', 'D')
    };

})(jQuery);