// Wait until domready
jQuery(document).ready(function($) {
    // On change submit the form with the updated limit count
    $('#sba-legislative-results-pager select').on('change', function() {

        // Get the stripped down form action.
        form_action = $(this).parents('#sba-legislative-results-pager').attr('action')+$(this).val();

        // Set the new form action with the limit count.
        $(this).parents('#sba-legislative-results-pager').attr('action', form_action);

        // Submit the form to reload the page.
        $(this).parents('#sba-legislative-results-pager').submit();
    });
});