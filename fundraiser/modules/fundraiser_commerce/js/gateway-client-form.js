Drupal.behaviors.fundraiserGatewayBehavior = {

  attach: function(context, settings) { (function($) {
    // Payment selection triggered changes.
    function paymentImages() {
      var gateway = $(this).val();
      var paymentId = this.id;
      var text = Drupal.settings.fundraiser[gateway].text;
      var labelImg = Drupal.settings.fundraiser[gateway].selected_image;
      // Automatically change submit button text when payment gateway selected.
      if (typeof(text) !== "undefined") {
        var oldText = $("#edit-submit").val().toUpperCase();
        $("#edit-submit").val(text);
        var fsm = $(".fundraiser_submit_message");
        if (fsm.length > 0) {
          var fsmHtml = fsm.html().replace(oldText, text.toUpperCase());
          fsm.html(fsmHtml);
        }
      }
      $('label[for='+paymentId+'] img').attr('src', labelImg);
      $("input[name='submitted[payment_information][payment_method]']").each(function(gateway) {
        if (!$(this).is(":checked")) {
          gateway = $(this).val();
          var paymentId = this.id;
          var labelImg = Drupal.settings.fundraiser[gateway].unselected_image;
          $('label[for='+paymentId+'] img').attr('src', labelImg);
        }
      });
    }

    // Change payment method image on hover
    function hoverInImage() {
      var gateway = $(this).attr('data-gateway');
      var paymentId = this.id;
      var labelImg = Drupal.settings.fundraiser[gateway].selected_image;
      $(this).attr('src', labelImg);
    }

    // Change payment method image after hover to selected default.
    function hoverOutImage() {
      var gateway = $(this).attr('data-gateway');
      var parentLabel = $(this).parent();
      var labelParent = parentLabel.attr('for');
      var checked = $('#' + labelParent).is(':checked');
      if (!checked) {
        var labelImg = Drupal.settings.fundraiser[gateway].unselected_image;
        $(this).attr('src', labelImg);
      }
    }

    // Call change to payment method images and submit button on page load and selection/hover.
    try {
      if (Drupal.settings.fundraiser.enabled_count >= 1) {
        // Set button text when page first loaded
        // Loop over payment method radio fields to find selected
        var paymentMethods = $('input[class*="fundraiser-payment-methods"]');
        var paymentMethodSelected;
        for(var i = 0; i < paymentMethods.length; i++){
          if(paymentMethods[i].checked){
            paymentMethodSelected = paymentMethods[i];
          }
        }
        if (paymentMethodSelected == undefined) {
          paymentMethodSelected = paymentMethods[0];
        }
        // Call our button/image change function with checked payment method on load.
        paymentImages.call(paymentMethodSelected);

        // Change payment option image based on payment selection and hover.
        $('input[class*="fundraiser-payment-methods"]').change(paymentImages);
      }
    } catch (e) {
      console.log(e);
    }
    // Show payment option "selected" images when rolling over.
    $('img[id*="payment-option-img"]').hover(hoverInImage, hoverOutImage);

  })(jQuery); }
};
