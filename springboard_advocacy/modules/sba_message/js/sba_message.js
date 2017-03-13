/**
 * @file
 */

/**
 * Advocacy recipients list  and search page
 */
(function ($) {
    
    // Functions which need to happen on initial page load, but not ajax reload
    $(document).ready(function () {

         // UI prep
         Sbam.messageFormUI();
        // Define click events for submit and delete buttons
        Sbam.messageFormSubmitter();

    });


    /*********  Function Definitions **************/

    //namespace
    window.Sbam = {};

    // move the checkbox to save a custom group below the Message Targets
     // container, and other UI prep.
     Sbam.messageFormUI = function() {
          if ($('#springboard-advocacy-find-targets-container')) {
               $('#edit-save-target-group').once(function(){
                    var groupSave = $('#edit-save-target-group').detach();
                    $('#springboard-advocacy-target-recipients > h3').after(groupSave);
               });
               // Events to populate the real hidden form elements with values from
               // the visible UI form inputs (because they need to be moved outside
               // the real form for UI presentation)
               $('#edit-save-target-group input').change(function(){
                    Sbam.messageFormUIPopHidden(this);
               });
               // Do it again after validation failure.
               $('#edit-save-target-group input').each(function(){
                    Sbam.messageFormUIPopHidden(this);
               });
          }
     };

     Sbam.messageFormUIPopHidden = function(item) {
          var id = $(item).attr('id');
          if($(item).attr('type') == 'checkbox') {
               if($(item).prop('checked')) {
                    $('input#' + id + '_val').val($(item).val());
               }
               else {
                    $('input#' + id + '_val').val(null);
               }
          }
          else {
               $('input#' + id + '_val').val($(item).val());
          }
     };

    //set up the event for the message save button validation and submit
    Sbam.messageFormSubmitter = function () {
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
         console.log('error trigger');
         Sba.setError(messages);
         }
         });
    };

})(jQuery);