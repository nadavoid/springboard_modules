This module provides functionality for sending analytics to Google's Measurement
Protocol service. Current events supported are:

- PayPal transactions (payments):
  Requires the googleanalytics and commerce_paypal modules.

In future, this module would preferrably also act as a bridging module between
your custom module and the Measurement Protocol service, by allowing you to
construct a payload and handling the transmission of the payload to Measurement
Protocol.
