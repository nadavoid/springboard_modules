INTRODUCTION
------------
This module defines a Drupal command "springboard-purge" that deletes older
records from the database with the intent to decrease size and increase
performance. It deletes the oldest records first, with the understanding
that Jenkins will be used to continually run this command until all old records
have been deleted.

Three arguments are accepted: users, webforms, or orders.

Three options are accepted: days-old, batch-size, protected-roles

DRUSH OPTIONS
---------------
* days-old
Defaults to 90 days. This drush command will not delete records that have been
created (or upddated, if applicable) less than `days-old` ago.

* batch-size
Defaults to 250. The number of records that will be deleted when the command is
executed.

* protected-roles
Applicable to `drush springboard-purge users` only. Allows you to specify
additional roles (besides those hard-coded in) to be protected from deletion.

DRUSH ARGUMENTS
-------------
# drush springboard-purge users
Users are deleted by way of invoking user_delete_multiple() so that appropriate
hooks are fired and other modules can act accordingly.

# drush springboard-purge webforms
This option deletes old webform submission. They are deleted by way of invoking
webform_submission_delete() so that appropirate hooks are fired and othe modules
can act accordingly.

In addiiton, `drush springboard-purge webforms` performs a check of the variable
`webform_confirmations_submissions_purge_enabled` and logs a warning if it is
not enabled. This configuration instructs the webform_confirmation module to
purge old confirmation permission records on cron. It should be enabled on
production sites.

# drush springboard-purge orders
This option deletes records from commerce_order, fundraiser_donation, and
fundraiser_sustainers tables. Commerce order records are deleted with
commerce_order_delete_multiple(). Associated fundraiser_sustainer records are
deleted with entity_delete_multiple(). Associated fundraiser_donation records
are deleted with a direct database query, because they are not entities and
donation_fundraiser_delete() when appears to do things we do not want done (such
as canceling orders in salesforce (salesforce_donation.module), and re-counting
the number of donations on its donation form)
