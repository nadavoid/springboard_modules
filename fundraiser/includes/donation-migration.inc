<?php

/**
 * The 
 */
abstract class FundraiserMigration extends Migration {
  public function __construct($arguments) {
    // Always call the parent constructor first for basic setup
    parent::__construct($arguments);

    // With migrate_ui enabled, migration pages will indicate people involved in
    // the particular migration, with their role and contact info. We default the
    // list in the shared class; it can be overridden for specific migrations.
    $this->team = array(
      new MigrateTeamMember('Jackson River', 'jr@jacksonriver.com', t('Migrator')),
    );
  }

  /**
   * Give the calling migration a shot at manipulating, and possibly rejecting,
   * the source row.
   *
   * @return bool
   *  FALSE if the row is to be skipped.
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Trim all the data.
    foreach ($row as $key => $value) {
      $row->{$key} = trim($value);
    }

    /**
     * Override this function to skip rows in your data.
     * 
     * For example the code below would pass over rows with a 
     * PAYMENT_DATE value less than a month in the past.
     * 
     * if (strtotime($row->{'PAYMENT_DATE'}) < strtotime('-1 month')) {
     *   return FALSE;
     * }
     */
  }

  /**
   * Example column header names. You would need at least these values to create donations.
   *
   * This should be overridden by your class and include all the column header's in your data.
   */
  public function csvColumnNames() {
    $cols = array(
      // A unique id will usually be required for matching the rows to the payment reference token.
      'UNIQUE_ID',
      'FIRST_NAME',
      'LAST_NAME',
      'EMAIL_ADDRESS',
      'PRIMARY_ADDRESS_LINE_1',
      'PRIMARY_ADDRESS_LINE_2',
      'PRIMARY_ADDRESS_CITY',
      'PRIMARY_ADDRESS_STATE',
      'PRIMARY_ADDRESS_ZIP_CODE',
      'PRIMARY_ADDRESS_COUNTRY',
      'PAYMENT_DATE',
      'PAYMENT_STATUS',
      'PAYMENT_AMOUNT',
      'FREQUENCY',
      'CREDIT_CARD_TYPE',
      'CREDIT_CARD_NUMBER',
      'EXPIRE_MONTH',
      'EXPIRE_YEAR',
      // This is usually provided separately from the main data dump by the payment processor.
      'PAYMENT_REFERENCE_TOKEN',
    );

    foreach ($cols as $col) {
      $csv_cols[] = array($col, $col);
    }

    return $csv_cols;
  }

  public function countryCode($name) {
    include_once DRUPAL_ROOT . '/includes/locale.inc';
    return array_search($name, country_get_list());
  }
}

/**
 * An example migration class.
 *
 * You probably want to copy this instead of extending this class as it
 * makes assumptions on the data and mapping options.
 */
class FundraiserDonationMigration extends FundraiserMigration {
  public function __construct($arguments) {
    // Always call the parent constructor first for basic setup
    parent::__construct($arguments);

    $this->description = t('Migrate donations');

    // The source ID here is the one retrieved from each data item in the XML file, and
    // used to identify specific items
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'csvrownum' => array(
          'type' => 'int',
          'not null' => TRUE,
        )
      ),
      FundraiserDonationDestination::getKeySchema()
    );

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->destination = new FundraiserDonationDestination();

    $this->addFieldMapping('recurs_monthly')->defaultValue(FALSE);

    // Add mappings.
    $this->addFieldMapping('mail', 'EMAIL_ADDRESS');
    $this->addFieldMapping('created')->defaultValue(date('c'));
    $this->addFieldMapping('amount', 'PAYMENT_AMOUNT');
    $this->addFieldMapping('payment_method', 'payment_method');
    $this->addFieldMapping('payment_fields', 'payment_fields');
    $this->addFieldMapping('cardonfile', 'cardonfile');
    $this->addFieldMapping('first_name', 'FIRST_NAME');
    $this->addFieldMapping('last_name', 'LAST_NAME');
    $this->addFieldMapping('state', 'PRIMARY_ADDRESS_STATE');
    $this->addFieldMapping('address', 'PRIMARY_ADDRESS_LINE_1');
    $this->addFieldMapping('address_line_2', 'PRIMARY_ADDRESS_LINE_2');
    $this->addFieldMapping('city', 'PRIMARY_ADDRESS_CITY');
    $this->addFieldMapping('zip', 'PRIMARY_ADDRESS_ZIP_CODE');
    $this->addFieldMapping('country', 'PRIMARY_ADDRESS_COUNTRY');
    $this->addFieldMapping('data', 'data');
    $this->addFieldMapping('currency')->defaultValue('USD');
    $this->addFieldMapping('status')->defaultValue(1);

    $this->addFieldMapping('nid')
      ->description(t('Enter the NID of the target donation form.'))
      ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_BLOCKER);

    $this->addFieldMapping('gateway') 
      ->description(t('Enter the ID of the commerce gateway to process these donations.'))
      ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_BLOCKER);
  }

  public function fields() {
    return array(
      'payment_method' => 'Payment method set in prepareRow()',
      'payment_fields' => 'An array of payment field data made during prepareRow().',
      'cardonfile' => 'An array of card data for a cardonfile record.',
      'data' => 'Data for the donation data array constructed during prepareRow().',
    );
  }

  /**
   * Give the calling migration a shot at manipulating, and possibly rejecting,
   * the source row.
   *
   * @return bool
   *  FALSE if the row is to be skipped.
   */
  public function prepareRow($row) {
    // Slow down migration
    sleep(1);

    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // These are all credit transactions
    $row->payment_method = 'credit';
    $row->payment_fields = $this->paymentFields($row);

    // Prepare data for card on file.
    $row->cardonfile = $this->cardOnFile($row);

    /**
     * Add an array of submission values.
     *
     * This would be the area to add additional submission values, like market source fields.
     */
    $row->submission = array();
    $row->submission['first_name'] = $row->{'FIRST_NAME'};
    $row->submission['last_name'] = $row->{'LAST_NAME'};
    $row->submission['address'] = $row->{'PRIMARY_ADDRESS_LINE_1'};
    $row->submission['address_line_2'] = $row->{'PRIMARY_ADDRESS_LINE_2'};
    $row->submission['city'] = $row->{'PRIMARY_ADDRESS_CITY'};
    $row->submission['state'] = $row->{'PRIMARY_ADDRESS_STATE'};
    $row->submission['zip'] = $row->{'PRIMARY_ADDRESS_ZIP_CODE'};
    $row->submission['country'] = $row->{'PRIMARY_ADDRESS_COUNTRY'};

    return TRUE;
  }

  public function paymentFields($row) {
    if ($row->payment_method == 'credit') {
      module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
      $fields['credit'] = array();
      // Card number.
      $fields['credit']['card_number'] = $row->{'CREDIT_CARD_NUMBER'};

      // Card exp date.
      $dates = explode('/', $row->{'Card Expiration Date'});
      $fields['credit']['card_expiration_month'] = $row->{'EXPIRE_MONTH'};
      $fields['credit']['card_expiration_year'] = $row->{'EXPIRE_YEAR'};

      // Card types, payflow records Discover.
      $types = array('discover' => 'Discover') + commerce_payment_credit_card_types();
      $type = array_search($row->{'CREDIT_CARD_TYPE'}, $types);
      $fields['credit']['card_type'] = !empty($type) ? $type : '';
    }

    return $fields;
  }

  public function cardOnFile($row) {
    $cardonfile = array(
      'remote_id' => $row->{'PAYMENT_REFERENCE_TOKEN'},
      'card_type' => $row->payment_fields['credit']['card_type'],
      'card_name' => substr($row->{'FIRST_NAME'}, 0, 120) . ' ' . substr($row->{'LAST_NAME'}, 0, 120),
      'card_number' => $row->payment_fields['credit']['card_number'],
      'card_exp_month' => $row->payment_fields['credit']['card_expiration_month'],
      'card_exp_year' => $row->payment_fields['credit']['card_expiration_year'],
      'status' => 1,
      'instance_default' => 1,
    );

    return $cardonfile;
  }
}

/**
 * Base class for creating user accounts from the list of donations.
 */
class FundraiserUserMigration extends FundraiserMigration {
  public function __construct($arguments) {
    // Always call the parent constructor first for basic setup
    parent::__construct($arguments);

    $this->description = t('Migrate Donation Users');
 
    // Create a MigrateSource object, which manages retrieving the input data.
    $this->destination = new MigrateDestinationUser();

    // Add the map
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'csvrownum' => array(
          'type' => 'int',
          'not null' => TRUE,
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );

    // This is a shortcut you can use to mark several destination fields as DNM
    // at once.
    $this->addUnmigratedDestinations(array(
      'access',
      'data',
      'language',
      'login',
      'picture',
      'role_names',
      'signature',
      'signature_format',
      'theme',
      'timezone',
    ));

    $this->addFieldMapping('roles')->defaultValue(DRUPAL_AUTHENTICATED_RID);

    // Add mappings
    $this->addFieldMapping('mail', 'EMAIL_ADDRESS');
    $this->addFieldMapping('name', 'EMAIL_ADDRESS');
    $this->addFieldMapping('status')->defaultValue(1);
    $this->addFieldMapping('sbp_first_name', 'FIRST_NAME');
    $this->addFieldMapping('sbp_last_name', 'LAST_NAME');
    $this->addFieldMapping('sbp_state', 'PRIMARY_ADDRESS_STATE');
    $this->addFieldMapping('sbp_address', 'PRIMARY_ADDRESS_LINE_1');
    $this->addFieldMapping('sbp_address_line_2', 'PRIMARY_ADDRESS_LINE_2');
    $this->addFieldMapping('sbp_city', 'PRIMARY_ADDRESS_CITY');
    $this->addFieldMapping('sbp_zip', 'PRIMARY_ADDRESS_ZIP_CODE');
    $this->addFieldMapping('sbp_country', 'PRIMARY_ADDRESS_COUNTRY');

  }

  /**
   * Give the calling migration a shot at manipulating, and possibly rejecting,
   * the source row.
   *
   * @return bool
   *  FALSE if the row is to be skipped.
   */
  public function prepareRow($row) {
    // $migration = Migration::currentMigration();
    // $migration->saveMessage($row->{'EMAIL_ADDRESS'});

    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $account = user_load_by_mail($row->{'EMAIL_ADDRESS'});

    // We do not want to mess with existing users.
    if (!empty($account)) {
      return FALSE;
    }

    return TRUE;
  }

  public function prepare(stdClass $account, stdClass $row) {
    $account->is_new = TRUE;

    // Cleanup the username.
    $username = preg_replace('/[^\x{80}-\x{F7} a-z0-9@_.\'-]/i', '-', trim($account->name));
    // We also limit the username to the maximum length for usernames.
    $account->name = substr($username, 0, USERNAME_MAX_LENGTH);
  }
}

/**
 * Extend the base classes for the test import.
 */
class FundraiserExampleDonation extends FundraiserDonationMigration {
  protected $dependencies = array('FundraiserExampleDonor');

  public function __construct($arguments) {
    // Always call the parent constructor first for basic setup.
    parent::__construct($arguments);

    /**
     * Example of using the private file directory.
     *
     * $wrapper = file_stream_wrapper_get_instance_by_uri('private://migrate');
     *
     * $folder = $wrapper->realpath();
     * $file = $folder . '/sustainer-import.csv';
     */

    $file = DRUPAL_ROOT . '/' . drupal_get_path('module', 'fundraiser') . '/includes/sustainer-import-example.csv';
    // Create a MigrateSource object, which manages retrieving the input data.
    // Notice we pass in the additional fields for the donation migration.
    $this->source = new MigrateSourceCSV($file, $this->csvColumnNames(), array('enclosure' => '"', 'header_rows' => 1), $this->fields());
  }

  /**
   * Give the calling migration a shot at manipulating, and possibly rejecting,
   * the source row.
   *
   * @return bool
   *  FALSE if the row is to be skipped.
   */
  public function prepareRow($row) {
    // Slow down migration.
    sleep(1);

    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // These are all credit transactions.
    $row->payment_method = 'credit';

    /**
     * You'll almost always need to play around with the payment date to
     * correct it.
     *
     * Since our example date will always be in the past we only use the
     * day value and reset to this month.
     */

    $start = strtotime($row->{'PAYMENT_DATE'});

    $day = date('d', $start);
    $month = date('m');
    $year = date('Y');

    $start = strtotime(date("$month/$day/$year"));

    /**
     * Below are important and unique variables for the import process.
     *
     * These are used in the donation destination class to control
     * some of the standard donation processing.
     */
    $row->recurring_start_date = $start;
    $row->create_sustainer_series = TRUE;

    // We don't want to fire the success hook.
    $row->donation_success = FALSE;

    // We do want to sync this donation.
    $row->send_to_queue = TRUE;
  }
}

class FundraiserExampleDonor extends FundraiserUserMigration {
  public function __construct($arguments) {
    // Always call the parent constructor first for basic setup.
    parent::__construct($arguments);
 
    /**
     * Example of using the private file directory.
     *
     * $wrapper = file_stream_wrapper_get_instance_by_uri('private://migrate');
     *
     * $folder = $wrapper->realpath();
     * $file = $folder . '/sustainer-import.csv';
     */

    $file = DRUPAL_ROOT . '/' . drupal_get_path('module', 'fundraiser') . '/includes/sustainer-import-example.csv';

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceCSV($file, $this->csvColumnNames(), array('enclosure' => '"', 'header_rows' => 1));
  }
}
