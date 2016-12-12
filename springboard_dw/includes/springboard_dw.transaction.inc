<?php

/**
 * @file
 * Transaction related functions for Springboard data warehouse.
 */

function springboard_dw_transaction_get_by_id($transaction_id) {
  $query = db_query('select * from {commerce_payment_transaction}
  where remote_id = :transaction_id',
    array(':transaction_id' => $transaction_id))->fetchAssoc();

  return $query;
}