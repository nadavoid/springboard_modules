<?php

/**
 * @file
 * Defines the class for connecting to the PSI payment service
 */

class PsiClient {
  /**
   * Constructor.
   */
  public function __construct($account) {

    // Determine the correct WSDL URL
    if ($account['txn_mode'] == COMMERCE_PSI_MODE_LIVE) {
      $this->wsdl = 'https://client.paymentsolutionsinc.net/Services/Payment/PsiWcfService.svc?wsdl';
    }
    elseif ($account['txn_mode'] == COMMERCE_PSI_MODE_STAGING) {
      $this->wsdl = 'https://client.paymentsolutionsinc.net/Staging/Services/Payment/PsiWcfService.svc?wsdl';
    }
    else {
      $this->wsdl = 'https://psiwebdev.paymentsolutionsincdev.net/Services/Payment/PsiWcfService.svc?wsdl';
    }

    // Create a new soapclient
    try {
      $options = array('connection_timeout' => 10);
      $this->soapclient = new SoapClient($this->wsdl, $options);
    }
    catch (Exception $ex) {
      watchdog('commerce_psi', 'Payment Services wsdl is not available.',
        array(),
        WATCHDOG_ERROR);
      return;
    }

    $this->account = $account;
  }

  /**
   * Return a setting for the account
   */
  public function getAccountSetting($name) {
    return !empty($this->account[$name]) ? $this->account[$name] : NULL;
  }

  /**
   * Returns the credentials array
   *
   * This must be included with every request
   */
  public function credentials() {
    return array(
      'credentials' => array(
        'UserName' => $this->getAccountSetting('account_username'),
        'Password' => $this->getAccountSetting('account_password'),
        'SourceIpAddress' => ip_address(),
      ),
    );
  }

  /**
   * Send a credit card request
   *
   * @param $payment_details Array
   *   An array of values to pass to the PSI API
   *
   * @return Object
   *   The resulting data from the request
   */
  public function creditRequest($payment_details) {

    $params = $this->credentials() + array('paymentInfo' => $payment_details);

    try {
      $response = $this->soapclient->SendCreditCardPayment($params);
    }
    catch (Exception $ex) {
      watchdog('commerce_psi', 'The PSI Commerce account @title failed to process an order: @error',
        array(
          '@title' => $this->getAccountSetting('title'),
          '@error' => $ex->getMessage(),
        ),
        WATCHDOG_ERROR);
    }

    if (!empty($response)) {
      return $response->SendCreditCardPaymentResult;
    }
  }

  /**
   * Send an electronic funds transfer request
   *
   * @param $payment_details Array
   *   An array of values to pass to the PSI API
   *
   * @return Object
   *   The resulting data from the request
   */
  public function eftRequest($payment_details) {

    $params = $this->credentials() + array('paymentInfo' => $payment_details);

    try {
      $response = $this->soapclient->SendEftPayment($params);
    }
    catch (Exception $ex) {
      watchdog('commerce_psi', 'The PSI Commerce account @title failed to process an order: @error',
        array(
          '@title' => $this->getAccountSetting('title'),
          '@error' => $ex->getMessage(),
        ),
        WATCHDOG_ERROR);
    }

    if (!empty($response)) {
      return $response->SendEftPaymentResult;
    }
  }

  public function TestConnection($message) {

    $params = $this->credentials() + array('message' => $message);

    try {
      $response = $this->soapclient->TestConnection($message);
    }
    catch (Exception $ex) {
      watchdog('commerce_psi', 'The PSI Commerce account @title failed to process an order: @error',
        array(
          '@title' => $this->getAccountSetting('title'),
          '@error' => $ex->getMessage(),
        ),
        WATCHDOG_ERROR);
    }

    if (!empty($response)) {
      return (object) array(
        'AmountApproved' => 10,
        'Approved' => 1,
        'AuthorizationCode' => 99999,
        'ClientTransactionId' => 100073006,
        'ProcessedDateTime' => '2013-05-17T08:44:53.87475-05:00',
        'ReasonCode' => 1,
        'ResponseCode' => 1,
        'ResponseMessage' => 'This was a test transaction.',
        'ResponseTime' => '2343.75',
        'TransactionId' => 123123123,
      );
    }
  }

  /**
   * Calls SendUpsell on the PSI API.
   *
   * @param array $upsell_data
   *   The values that the SendUpsell API call expects.
   *
   * @todo Finish this.
   */
  public function SendUpsell($upsell_data) {

    $params = $this->credentials() + array('upsellInfo' => $upsell_data);
    try {
      $response = $this->soapclient->SendUpsell($params);
    }
    catch (Exception $ex) {
      watchdog('commerce_psi', 'The PSI Commerce account @title failed to send an upsell: @error',
        array(
          '@title' => $this->getAccountSetting('title'),
          '@error' => $ex->getMessage(),
        ),
        WATCHDOG_ERROR);
    }

    if (!empty($response)) {
      return $response->SendUpsellResult;
    }

    return NULL;
  }

}
