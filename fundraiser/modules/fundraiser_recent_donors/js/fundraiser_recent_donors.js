/**
 * @file
 * js for Fundraiser Recent Donors
 */

Drupal.behaviors.fundraiserRecentDonorsBehavior = {
  attach: function(context) { (function($) {
    // Position the opt-in checkbox near the submit button:

    // Position currency suffix/prefix inline with the "Only show donations greater than" field:
    $('.form-item-show-recent-donation-greater-than input#edit-show-recent-donation-greater-than').after($('span.recent-donors-currency-symbol-after'));
    $('.form-item-show-recent-donation-greater-than input#edit-show-recent-donation-greater-than').before($('span.recent-donors-currency-symbol-before'));
  })(jQuery); }
}

