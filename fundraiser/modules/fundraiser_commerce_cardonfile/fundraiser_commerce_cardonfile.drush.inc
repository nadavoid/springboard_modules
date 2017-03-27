<?php
/**
 * @file
 * Drush commands for the fundraiser_commerce_cardonfile module.
 */

const FUNDRAISER_COMMERCE_CARDONFILE_CARD_DRUSH_DESCRIPTION = 'Processes cardonfile cards to set their appropriate adress profile_id values.';
const FUNDRAISER_COMMERCE_CARDONFILE_DONATION_DRUSH_DESCRIPTION = 'Processes donations to set their appropriate card_id values.';

/**
 * Implements hook_drush_help().
 */
function fundraiser_commerce_cardonfile_drush_help($command) {
  switch ($command) {
    case 'drush:fundraiser-commerce-cardonfile-card-process':
      return dt(FUNDRAISER_COMMERCE_CARDONFILE_DRUSH_DESCRIPTION);

    case 'drush:fundraiser-commerce-cardonfile-donation-process':
      return dt(FUNDRAISER_COMMERCE_CARDONFILE_DONATION_DRUSH_DESCRIPTION);
  }
}

/**
 * Implements hook_drush_command().
 */
function fundraiser_commerce_cardonfile_drush_command() {
  $items = array();
  $items['fundraiser-commerce-cardonfile-card-process'] = array(
    'description' => FUNDRAISER_COMMERCE_CARDONFILE_CARD_DRUSH_DESCRIPTION,
    'options' => array(
      'limit' => 'The maximum number of cards to process.',
    ),
    'examples' => array(
      'Standard example' => 'fundraiser-commerce-cardonfile-card-process',
    ),
    'aliases' => array('fcccp'),
  );
  $items['fundraiser-commerce-cardonfile-donation-process'] = array(
    'description' => FUNDRAISER_COMMERCE_CARDONFILE_DONATION_DRUSH_DESCRIPTION,
    'options' => array(
      'limit' => 'The maximum number of cards to process.',
    ),
    'examples' => array(
      'Standard example' => 'fundraiser-commerce-cardonfile-donation-process',
    ),
    'aliases' => array('fccdp'),
  );

  return $items;
}

/**
 * Drush command to process recurring cards.
 */
function drush_fundraiser_commerce_cardonfile_card_process() {
  $limit = drush_get_option('limit', 200);

  module_load_include('inc', 'fundraiser_commerce_cardonfile', 'includes/fundraiser_commerce_cardonfile.admin');
  $context = _fundraiser_commerce_cardonfile_update_card_addresses($limit);

  if (!empty($context['progress']) && !empty($context['max'])) {
    return 'Processed ' . $context['progress'] . ' of ' . $context['max'] . ' cards.';
  }
  else {
    return 'No cards needed processing.';
  }
}

/**
 * Drush command to process recurring donations.
 */
function drush_fundraiser_commerce_cardonfile_donation_process() {
  $limit = drush_get_option('limit', 200);

  module_load_include('inc', 'fundraiser_commerce_cardonfile', 'includes/fundraiser_commerce_cardonfile.admin');
  $context = _fundraiser_commerce_cardonfile_update_donation_cardids($limit);

  if (!empty($context['progress']) && !empty($context['max'])) {
    return 'Processed ' . $context['progress'] . ' of ' . $context['max'] . ' donations.';
  }
  else {
    return 'No donations needed processing.';
  }
}
