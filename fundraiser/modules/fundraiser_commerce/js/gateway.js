Drupal.behaviors.fundraiserGateway = {
  attach: function (context) {
    (function ($) {
      $(document).ready(function () {
        $("#edit-submit").once('processed', function () {
          $("#edit-submit").click(function () {
            return confirm("Please review your changes carefully. A misconfiguration can cause transactions to fail.\n\nAre you sure you want to proceed?");
          });
        });
      });
    })(jQuery);
  }
};
