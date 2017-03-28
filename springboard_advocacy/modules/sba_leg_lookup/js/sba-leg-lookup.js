/**
 * @file
 * Shpws/Hides legislator lookup results
 */
(function ($) {

  /*
   * Gets Form build info from settings and adds it to ajax data.
   *
   * @see springboard_advocacy_views_exposed_form_ajax_enable().
   */
  Drupal.behaviors.SbaLegLookup = {
    attach: function (context, settings) {
      $('#sba-legislators').once('toggle-processed', function() {
      $('.leg-lookup-show').each(function () {
        console.log(this);
        $(this).click(function () {
          if (!$(this).hasClass('lookup-processed')) {
            $(this).closest('.sba-leg-lookup-expand').siblings('.leg-lookup-hidden').show('slow');
            $(this).text('Show less');
            $(this).addClass('lookup-processed');
          }
          else {
            $(this).closest('.sba-leg-lookup-expand').siblings('.leg-lookup-hidden').hide('slow');
            $(this).text('Show more');
            $(this).removeClass('lookup-processed');
          }
          return false;
        });
      });
      });
    }
  };

})(jQuery);