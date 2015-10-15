<?php

/**
 * @file
 * The controller for the donation entity containing the CRUD operations.
 */

/**
 * The controller class for donations contains methods for the donation CRUD
 * operations. The load method is inherited from the default controller.
 */
class FundraiserDonationEntityController extends EntityAPIController {

  /**
   * Create a default donation entity.
   *
   * @param array $values
   *   An array of values to set, keyed by property name.
   *
   * @return
   *   An donatoin object with all default fields initialized.
   */
  public function create(array $values = array()) {
    if (!isset($values['uid'])) {
      $account = $GLOBALS['user'];
      $values['uid'] = $account->uid;
    }

    $values += array(
      'nid' => 0,
      'sid' => 0,
      'status' => 1,
      'amount' => NULL,
      'currency' => '',
      'txn_id' => '',
      'form_url' => '',
      'created' => NULL,
      'changed' => NULL,
      'gateway' => '',
    );

    return parent::create($values);
  }

  /**
   * Saves an donation entity.
   *
   * Saves the donation. Runs before _fundraiser_create_donation which saves the non-field data.
   * _fundraiser_create_donation checks for an existing record so we don't have to worry about id collisions
   *
   * @param $fundraiser_donation
   *   The full donation object to save.
   *
   * @return
   *   SAVED_NEW or SAVED_UPDATED depending on the operation performed.
   */
  public function save($fundraiser_donation) {
    $transaction = db_transaction();

    try {
      global $user;

      // Determine if we will be inserting a new donation. 
      // The id for the donations are set by Commerce; we have to check the table to ensure its new
      if (!empty($fundraiser_donation->is_new) && !empty($fundraiser_donation->did))  {
        $db_exist = db_query('SELECT did FROM {fundraiser_donation} WHERE did = :did', array(':did' => $fundraiser_donation->did))->fetchField();
      }

      if (!empty($fundraiser_donation->did) && !isset($fundraiser_donation->original)) {
        $fundraiser_donation->original = entity_load_unchanged('fundraiser_donation', $fundraiser_donation->did);
      }

      field_attach_presave('fundraiser_donation', $fundraiser_donation);

      // Let modules modify the donation entity before it is saved to the database. 
      module_invoke_all('fundraiser_entity_donation_presave', $fundraiser_donation);
      module_invoke_all('entity_presave', $fundraiser_donation, 'fundraiser_donation');

      // Ensure the donation value is the id string
      if (is_array($fundraiser_donation->gateway)) {
        $fundraiser_donation->gateway = $fundraiser_donation->gateway['id'];
      }

      // Set the timestamp fields.
      if (!empty($fundraiser_donation->is_new) && empty($db_exist)) {
        $fundraiser_donation->created = REQUEST_TIME;
      }
      else {
        // Otherwise if the donation is not new but comes from an entity_create()
        // or similar function call that initializes the created timestamp, uid,
        // and hostname values to empty strings, unset them to prevent
        // destroying existing data in those properties on update.
        if ($fundraiser_donation->created === '') {
          unset($fundraiser_donation->created);
        }
        if ($fundraiser_donation->uid === '') {
          unset($fundraiser_donation->uid);
        }
        if ($fundraiser_donation->nid === '') {
          unset($fundraiser_donation->nid);
        }
        if ($fundraiser_donation->sid === '') {
          unset($fundraiser_donation->sid);
        }
      }

      $fundraiser_donation->changed = REQUEST_TIME;

      $op = !empty($fundraiser_donation->is_new) ? 'insert' : 'update';

      // Save the donation record if it hasn't been written already
      if (empty($fundraiser_donation->record_written)) {
        // No record in the table, insert
        if ($op == 'insert' && empty($db_exist)) {
          $return = drupal_write_record('fundraiser_donation', $fundraiser_donation);
        }
        // Existing record so update
        else {
          $return = drupal_write_record('fundraiser_donation', $fundraiser_donation, 'did');
        }
      }
      else {
        // If the records been written we return the updated flag
        $return = SAVED_UPDATED;
      }

      // Save fields.
      $function = "field_attach_$op";
      $function('fundraiser_donation', $fundraiser_donation);

      // Let modules modify the donation entity after it is saved to the database.
      module_invoke_all('fundraiser_entity_donation_' . $op, $fundraiser_donation);
      module_invoke_all('entity_' . $op, $fundraiser_donation, 'fundraiser_donation');

      unset($fundraiser_donation->is_new);
      unset($fundraiser_donation->original);
      // Clear the static loading cache.
      entity_get_controller('fundraiser_donation')->resetCache(array(($fundraiser_donation->did)));

      // Ignore slave server temporarily to give time for the
      // saved data to be propagated to the slave.
      db_ignore_slave();

      return $return;
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('fundraiser_entity', $e);
      throw $e;
    }
  }

  /**
   * Allow modules to add data to the donation object
   */
  public function attachLoad(&$queried_entities, $revision_id = false) {
    foreach ($queried_entities as $did => &$donation) {
      // To be compatible with objects loaded with fundraiser_donation_get_donation set some additional values.
      $donation->node = node_load($donation->nid);
      $donation->user = user_load($donation->uid);

      // Add refund data
      $donation->refunds = FALSE;
      $donation->refunds = fundraiser_refund_get_refunds_by_did($did, TRUE);

      // Set the gateway to be the array of gateway info
      $donation->gateway = _fundraiser_gateway_info($donation->gateway);

      // Allow other modules to add data.
      module_invoke_all('fundraiser_donation_get_donation', $donation);
    }

    // Call the default attachLoad() method. This will add fields
    parent::attachLoad($queried_entities);
  }

  /**
   * Builds a structured array representing the entity's content.
   *
   * The content built for the entity will vary depending on the $view_mode
   * parameter.
   *
   * @param $entity
   *   An entity object.
   * @param $view_mode
   *   View mode, e.g. 'full', 'teaser'...
   * @param $langcode
   *   (optional) A language code to use for rendering. Defaults to the global
   *   content language of the current request.
   * @return
   *   The renderable array.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    // Remove previously built content, if exists.
    $entity->content = $content;
    $langcode = isset($langcode) ? $langcode : $GLOBALS['language_content']->language;

    // Add in fields.
    if (!empty($this->entityInfo['fieldable'])) {
      $entity->content += field_attach_view($this->entityType, $entity, $view_mode, $langcode);
    }

    $entity->content['donation_information'] = array(
      '#prefix' => '<div class="field field-name-fundraiser-donation field-type-fundraiser-donation field-label-above">' .
        '<div class="field-label">' . t('Donation information') . ': </div>' .
            '<div class="field-items"><div class="field-item even">',
      '#markup' => fundraiser_donation_information_table($entity->did),
      '#suffix' => '</div></div></div>',
    );

    $entity->content['status'] = array(
      '#prefix' => '<div class="field field-name-fundraiser-donation field-type-fundraiser-donation field-label-above">' .
        '<div class="field-label">' . t('Status') . ': </div>' .
            '<div class="field-items"><div class="field-item even">',
      '#markup' => check_plain($entity->status_label),
      '#suffix' => '</div></div></div>',
    );

    $entity->content['gateway'] = array(
      '#prefix' => '<div class="field field-name-fundraiser-donation field-type-fundraiser-donation field-label-above">' .
        '<div class="field-label">' . t('Gateway') . ': </div>' .
            '<div class="field-items"><div class="field-item even">',
      '#markup' => check_plain($entity->gateway['gateway_details']['title']),
      '#suffix' => '</div></div></div>',
    );

    $entity->content['txn_id'] = array(
      '#prefix' => '<div class="field field-name-fundraiser-donation field-type-fundraiser-donation field-label-above">' .
        '<div class="field-label">' . t('Transaction ID') . ': </div>' .
            '<div class="field-items"><div class="field-item even">',
      '#markup' => check_plain($entity->txn_id),
      '#suffix' => '</div></div></div>',
    );

    // Invoke hook_ENTITY_view() to allow modules to add their additions.
    module_invoke_all($this->entityType . '_entity_view', $entity, $view_mode, $langcode);

    // Invoke the more generic hook_entity_view() to allow the same.
    module_invoke_all('entity_view', $entity, $this->entityType, $view_mode, $langcode);

    // Remove the build array information from the entity and return it.
    $build = $entity->content;
    unset($entity->content);

    return $build;
  }

  /**
   * Generate an array for rendering the given entities.
   *
   * @param $entities
   *   An array of entities to render.
   * @param $view_mode
   *   View mode, e.g. 'administrator'...
   * @param $langcode
   *   (optional) A language code to use for rendering. Defaults to the global
   *   content language of the current request.
   * @param $page
   *   (optional) If set will control if the entity is rendered: if TRUE
   *   the entity will be rendered without its title, so that it can be embeded
   *   in another context. If FALSE the entity will be displayed with its title
   *   in a mode suitable for lists.
   *   If unset, the page mode will be enabled if the current path is the URI
   *   of the entity, as returned by entity_uri().
   *   This parameter is only supported for entities which controller is a
   *   EntityAPIControllerInterface.
   * @return
   *   The renderable array.
   */
  public function view($entities, $view_mode = '', $langcode = NULL, $page = NULL) {
    // Create a new entities array keyed by entity ID.
    $rekeyed_entities = array();

    foreach ($entities as $key => $entity) {
      // Use the entity's ID if available and fallback to its existing key value
      // if we couldn't determine it.
      if (isset($entity->{$this->idKey})) {
        $key = $entity->{$this->idKey};
      }

      $rekeyed_entities[$key] = $entity;
    }

    $entities = $rekeyed_entities;

    // If no view mode is specified, use the first one available..
    if (!isset($this->entityInfo['view modes'][$view_mode])) {
      reset($this->entityInfo['view modes']);
      $view_mode = key($this->entityInfo['view modes']);
    }

    if (!empty($this->entityInfo['fieldable'])) {
      field_attach_prepare_view($this->entityType, $entities, $view_mode);
    }

    entity_prepare_view($this->entityType, $entities);
    $langcode = isset($langcode) ? $langcode : $GLOBALS['language_content']->language;
    $view = array();

    // Build the content array for each entity passed in.
    foreach ($entities as $key => $entity) {
      $build = entity_build_content($this->entityType, $entity, $view_mode, $langcode);

      // Add default properties to the array to ensure the content is passed
      // through the theme layer.
      $build += array(
        '#theme' => 'entity',
        '#entity_type' => $this->entityType,
        '#entity' => $entity,
        '#view_mode' => $view_mode,
        '#language' => $langcode,
        '#page' => $page,
      );

      // Allow modules to modify the structured entity.
      drupal_alter(array($this->entityType . '_view', 'entity_view'), $build, $this->entityType);
      $view[$this->entityType][$key] = $build;
    }

    return $view;
  }
}
