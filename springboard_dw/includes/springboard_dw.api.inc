<?php
/**
 * @file
 * API functions for Springboard data warehouse.
 */

// Use the autoloaded Guzzle client and client exception handlers.
use GuzzleHttp\Client as guzzle_client;
use GuzzleHttp\Exception\ClientException as guzzle_client_exception;

/**
 * Returns a new instance of Guzzle Http client.
 *
 * @return \GuzzleHttp\Client $guzzle
 *   Return an instance of the Guzzle client.
 */
function springboard_dw_api_client() {
  // Composer Vendor autoload.
  if (function_exists('drush_autoload')) {
    drush_autoload("sites/all/libraries/springboard_composer/vendor/autoload.php");
  }
  else {
    include_once "sites/all/libraries/springboard_composer/vendor/autoload.php";
  }

  // Load Guzzle client.
  $guzzle = new guzzle_client(['http_errors' => FALSE, 'verify' => FALSE]);

  // Return an instance of the client.
  return $guzzle;
}

/**
 * Returns the endpoint for the data warehouse receiver app.
 *
 * @param string $endpoint_route
 *   The endpoint route to use in constructing the full URL for the api call.
 *
 * @return string
 *   The full api endpoint.
 */
function springboard_dw_api_endpoint($endpoint_route) {
  $api_endpoint = variable_get('springboard_dw_api_endpoint', NULL);

  return $api_endpoint . '/' . ltrim($endpoint_route, '/');
}

/**
 * Removes the queue type, adds auth and preps api call.
 *
 * @param array $item
 *   An array of values.
 *
 * @return array $item
 *   An array of values with authentication and client identification
 *   parameters added.
 */
function springboard_dw_api_prep_call($item) {
  // Remove the queue type from the item.
  unset($item['queue_type']);

  // Add the authentication token and client id.
  $item['authenticationToken'] = variable_get('springboard_dw_authentication_token', NULL);

  return $item;
}

/**
 * Make an api call to the warehouse providing endpoint and form params.
 *
 * @param string $endpoint_route
 *   The endpoint route used to construct the full URI for the API call.
 * @param array $params
 *   The parameters used in the API call added to the URI for debugging
 *   purposes.
 * @param string $method
 *   The HTTP verb/method to use for the API call (defaults to POST).
 *
 * @return mixed|\Psr\Http\Message\ResponseInterface
 *   Return the api response
 */
function springboard_dw_api_call($endpoint_route, $params, $method = 'POST') {
  // Define the endpoint.
  $endpoint = springboard_dw_api_endpoint($endpoint_route);

  // Instantiate an API client instance.
  $api_client = springboard_dw_api_client();

  // Clean up the item and add authentication to the form params.
  $params = springboard_dw_api_prep_call($params);

  try {
    // Attempt Guzzle client request.
    $api_response = $api_client->request(
      $method, $endpoint, array(
        'form_params' => $params,
      )
    );

    // Setup an array of the API call details prepped for logging.
    $api_call['endpoint'] = $endpoint;
    $api_call['method'] = $method;
    $api_call['params'] = $params;
    $api_call['response'] = $api_response->getBody()->getContents();
    $api_call['responseCode'] = $api_response->getStatusCode();
    $api_call['fullUrl'] = $endpoint . '?' . http_build_query(springboard_dw_api_prep_call
      ($api_call['params']));

    $watchdog_string = "<pre>Warehouse API call: \n" . print_r($api_call, TRUE) . "</pre>";

    // Log the full API call details to watchdog if logging is enabled.
    springboard_dw_watchdog_log($watchdog_string);

    return $api_response;
  }
  catch (guzzle_client_exception $e) {
    // Log the full API call failure details to watchdog if logging is enabled.
    $watchdog_string = '<pre>' . print_r($e, TRUE) . '</pre>';
    springboard_dw_watchdog_log($watchdog_string);
  }
}

/**
 * Mark an item in Drupal as being warehoused.
 *
 * @param string $type
 *   The type or identifier for what kind of item we are tracking.
 * @param int $identifier
 *   The unique identifier for the item (typically a Drupal ID unless it is a
 *   message item in which case it is a MongoId string).
 *
 * @throws \Exception
 */
function springboard_dw_track_item_warehouse_success($type, $identifier) {
  // Insert the user account into the Drupal data warehouse table.
  db_insert('springboard_data_warehouse')->fields(
    array(
      'type'      => $type,
      'drupal_id' => $identifier,
      'created'   => time(),
    )
  )->execute();
}
