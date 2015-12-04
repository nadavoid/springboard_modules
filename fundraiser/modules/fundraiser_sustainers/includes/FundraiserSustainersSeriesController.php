<?php

/**
 * @file
 * The controller for the fundraiser sustainers series entity.
 */
class FundraiserSustainersSeriesEntityController extends EntityAPIController {

  /**
   * Saves a fundraiser sustainers series entity.
   *
   * @param Entity $fundraiser_sustainers_series
   *   The fundraiser sustainers series to save.
   *
   * @return Entity
   *   The saved fundraiser sustainers series.
   */
  public function save($fundraiser_sustainers_series) {
    $fundraiser_sustainers_series->updated = time();

    if (isset($fundraiser_sustainers_series->is_new) && $fundraiser_sustainers_series->is_new) {
      $fundraiser_sustainers_series->created = time();
    }

    parent::save($fundraiser_sustainers_series);

    return $fundraiser_sustainers_series;
  }

}
