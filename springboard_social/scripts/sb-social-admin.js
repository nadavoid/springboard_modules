(function ($) {

  Drupal.behaviors.sb_social_admin = {
    attach: function (context, settings) {
      $("#edit-submit").click(function() {
      var addThisProf = $('#edit-springboard-social-addthis-profile-id');
        if (addThisProf.is(':visible')) {
          if (addThisProf.val() === '') {
            $('#addthis-error').remove();
            addThisProf.closest('.control-group').addClass('error');
            addThisProf.closest('.control-group').css({'background-color' : '#fff', 'color' : '#b94a48'});
            addThisProf.closest('.control-group').append('<label id="addthis-error" generated="true">Can\'t be empty when AddThis is enabled.</label>');
            return false;
          }
        }
      });
    }
  };
})(jQuery);