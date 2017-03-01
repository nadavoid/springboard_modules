<?php


/**
 * Provides content for the legislator lookup block.
 */
function sba_leg_lookup_legislator_lookup($delta, $mb_delta) {
  switch ($delta) {
    case 'sba_legislator_lookup':
      return drupal_get_form('sba_leg_lookup_legislator_lookup_form', $mb_delta);
  }
  return NULL;
}

/**
 * Theme function to render list of legislators.
 */
function sba_leg_lookup_preprocess_sba_leg_lookup_legislators(&$vars) {
  $legislator_roles = sba_leg_lookup_legislator_roles();
  $vars['legislators_proecessed'] = array();
  $vars['phone_only'] = variable_get('sba_legislator_lookup_phone_' . $vars['delta'], 0);
  foreach ($vars['legislators'] as $role => $legislators) {
    $label = $legislator_roles[$role];
    if (in_array($role, array('FR', "FS", 'SR', 'SS'))) {
      $num = count($legislators);
      $label = format_plural($num, $legislator_roles[$role], $legislator_roles[$role] . 's');
    }
    $vars['legislators_processed'][$role]['label'] = $label;
    foreach ($legislators as $legislator) {
      $vars['legislators_processed'][$role]['legislators'][] = sba_leg_lookup_legislator_process($legislator, $vars['delta']);
    }
  }
}

/**
 * Creates the HTML markup to render a single legislator.
 */
function sba_leg_lookup_legislator_process($legislator, $delta) {

  $leg_arr = array();

  // Set name variable.
  $leg_arr['name'] = $legislator->salutation . ' ' . $legislator->first_name . ' ' . $legislator->last_name . ' (' . $legislator->party . ')';

  // Set district name variable.
  if (!in_array($legislator->role, array('PRES01', 'PRES03', 'GOVNR'))) {
    if ($legislator->role == 'FS') {
      $district_name = str_replace('US Senate', '', $legislator->district_name);
    }
    else {
      $district_name = $legislator->district_name;
    }
    $leg_arr['role'] = $legislator->title . ', ' . $district_name;
  }
  else {
    $leg_arr['role'] = $legislator->title;
  }

  // Process addresses.
  $leg_arr['addresses'] = array();
  if (!empty($legislator->addresses)) {

    $leg_arr['office_label'] = t('Office');
    $leg_arr['main_phone'] = '';
    $leg_arr['main_fax'] = '';

    foreach ($legislator->addresses as $address) {
      switch ($address->type) {
        case 'Fed Capitol':
          $type = t('Capitol');
          break;

        case 'Fed District':
          $type = t('District');
          if (!empty($address->phone)) {
            $leg_arr['main_phone'] = $address->area_code . '-' . $address->phone;
          }
          if (!empty($address->fax)) {
            $leg_arr['main_fax'] = $address->fax_area_code . '-' . $address->fax;
          }

          break;

        case 'State District':
          $type = t('District');
          break;

        case 'State Capitol':
          $type = t('Capitol');
          if (!empty($address->phone)) {
            $leg_arr['main_phone'] = $address->area_code . '-' . $address->phone;
          }
          if (!empty($address->fax)) {
            $leg_arr['main_fax'] = $address->fax_area_code . '-' . $address->fax;
          }
          break;

        case 'State Exec Office':
          $type = t('Capitol');
          if (!empty($address->phone)) {
            $leg_arr['main_phone'] = $address->area_code . '-' . $address->phone;
          }
          if (!empty($address->fax)) {
            $leg_arr['main_fax'] = $address->fax_area_code . '-' . $address->fax;
          }
          break;

        case 'Fed Exec Office':
          $type = t('Capitol');
          if (!empty($address->phone)) {
            $leg_arr['main_phone'] = $address->area_code . '-' . $address->phone;
          }
          if (!empty($address->fax)) {
            $leg_arr['main_fax'] = $address->fax_area_code . '-' . $address->fax;
          }
          break;

        default:
          $type = FALSE;
          break;
      }

      if (!empty($type)) {
        $leg_arr['addresses'][$type]['office_type'] = $type;
        $street = '';
        if (!empty($address->building)) {
          $street = $address->building . '<br/>';
        }
        $street .= $address->street . '<br/>';
        $street .= $address->city . ', ' . $address->state . ' ' . $address->zip_code . '<br/>';
        $leg_arr['addresses'][$type]['street'] = $street;

        if (!empty($address->phone)) {
          $leg_arr['addresses'][$type]['phone'] = $address->area_code . '-' . $address->phone;
        }
        if (!empty($address->fax)) {
          $leg_arr['addresses'][$type]['fax'] = $address->fax_area_code . '-' . $address->fax;
        }
      }
    }
  }

  if (!empty($leg_arr['addresses'])) {
    krsort($leg_arr['addresses']);
  }

  if (!empty($legislator->email)) {
    $leg_arr['email'] = $legislator->email;
  }
  if (!empty($legislator->contact_form)) {
    $leg_arr['contact_form'] = $legislator->contact_form;
  }
  else {
    if (!empty($legislator->website)) {
      $leg_arr['website'] = $legislator->website;
    }
  }

  if (!empty($legislator->facebook)) {
    $leg_arr['facebook'] = $legislator->facebook;
  }

  if (!empty($legislator->twitter)) {
    $leg_arr['twitter'] = $legislator->twitter;
  }
  if (!empty($legislator->youtube)) {
    $leg_arr['youtube'] = $legislator->youtube;
  }

  if (!empty($legislator->committee_memberships)) {
    foreach ($legislator->committee_memberships as $com) {
      $leg_arr['committees'][] = array('name' => $com->committee_name);
    }
  }

  return $leg_arr;
}

/**
 * Block content for sba_legislator_lookup block.
 *
 * Displays a form for users to enter their address and perform a legislator
 * lookup. After successful submission the form is hidden and a list of
 * elected officials id displayed in it's place.
 */
function sba_leg_lookup_legislator_lookup_form($form, &$form_state, $mb_delta) {
  $form['#attached']['css'][] = drupal_get_path('module', 'sba_leg_lookup') . '/css/lookup.css';

  // If form has been submitted and we have some legislators, show them.
  if (!empty($form_state['legislators'])) {
    $form['#attached']['js'][] = drupal_get_path('module', 'sba_leg_lookup') . '/js/sba-leg-lookup.js';
    if ($form_state['legislators'] != 'no response') {
      $form['submission'] = array(
        '#markup' => theme('sba_leg_lookup_legislators', array(
          'legislators' => $form_state['legislators'],
          'delta' => $mb_delta,
        )),
      );
    }
    else {
      $form['submission'] = array(
        '#markup' => t('There was a problem looking up your address. Please check your address or try again later.'),
      );
    }
  }
  else {
    $form['sbp_address'] = array(
      '#type' => 'textfield',
      '#title' => t('Street Address'),
      '#description' => t('Do not include apartment or suite numbers.'),
      '#required' => TRUE,
    );

    $form['sbp_city'] = array(
      '#type' => 'textfield',
      '#title' => t('City'),
      '#required' => TRUE,
    );

    module_load_include('inc', 'springboard_target', 'includes/springboard_target.custom_targets');
    $form['sbp_state'] = array(
      '#type' => 'select',
      '#title' => t('State'),
      '#options' => sba_leg_lookup_states(),
      '#required' => TRUE,
      '#maxlength' => 2,
    );

    $form['sbp_zip'] = array(
      '#type' => 'textfield',
      '#title' => t('ZIP Code'),
      '#required' => TRUE,
      '#maxlength' => 5,
    );

    $form['mb_delta'] = array(
      '#type' => 'hidden',
      '#value' => $mb_delta,
    );

    $form['plus4'] = array(
      '#type' => 'hidden',
      '#value' => '',
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => variable_get('sba_legislator_lookup_submit_text_' . $mb_delta, array('Find my legislators')),
    );
  }

  return $form;
}

/**
 * Validation handler for sba_leg_lookup_legislator_lookup_form.
 *
 * Performs address validation and if successful stashes the plus4
 * in form_state to be used in the submission handler.
 */
function sba_leg_lookup_legislator_lookup_form_validate($form, &$form_state) {
  form_load_include($form_state, 'inc', 'springboard_advocacy', 'includes/springboard_advocacy.smarty_streets');

  $form_error = &drupal_static('form_set_error');
  if (empty($form_error)) {
    $geo = springboard_advocacy_smarty_lookup($form_state['values']);
  }

  if (!empty($geo) && !empty($geo['zip']) && !empty($geo['plus4'])) {
    form_set_value($form['sbp_zip'], $geo['zip'], $form_state);
    form_set_value($form['plus4'], $geo['plus4'], $form_state);
  }
  else {
    form_set_error('sbp_address', 'Unable to verify the submitted address. Please try again.');
  }
}

/**
 * Submit handler for sba_leg_lookup_legislator_lookup_form.
 *
 * Invokes the advocacy server getLegislator method and stashes the
 * result in form_state to user in the form display.
 */
function sba_leg_lookup_legislator_lookup_form_submit($form, &$form_state) {
  $full_zip = $form_state['values']['sbp_zip'] . '-' . $form_state['values']['plus4'];
  $api_call = new AdvocacyApiCall();
  $response = $api_call->invokeClientMethod('getLegislators', $full_zip);
  if (isset($response->data) && $response->status_code == 200) {
    $form_state['legislators'] = sba_leg_lookup_sort_legislators(get_object_vars($response->data), $form_state['values']['mb_delta']);
  }
  else {
    $form_state['legislators'] = 'no response';
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * Breaks legislators down by role.
 *
 * Prunes out roles according to the
 * block settings, and sorts according to fed/state levels.
 */
function sba_leg_lookup_sort_legislators($legislators, $mb_delta) {
  $sorted_legislators = array();
  $sba_legislator_lookup_types = variable_get('sba_legislator_lookup_types_' . $mb_delta,
    array('FS', 'FR', 'SS', 'SR', 'GOVNR', 'PRES01', 'PRES03')
  );

  foreach ($legislators as $legislator) {
    if (in_array($legislator->role, $sba_legislator_lookup_types)) {
      switch ($legislator->role) {

        case 'FS':
          $sorted_legislators[0][$legislator->role][] = $legislator;
          break;

        case 'FR':
          $sorted_legislators[1][$legislator->role][] = $legislator;
          break;

        case 'PRES01':
          $sorted_legislators[2][$legislator->role][] = $legislator;
          break;

        case 'PRES03':
          $sorted_legislators[3][$legislator->role][] = $legislator;
          break;

        case 'GOVNR':
          $sorted_legislators[4][$legislator->role][] = $legislator;
          break;

        case 'SS':
          $sorted_legislators[5][$legislator->role][] = $legislator;
          break;

        case 'SR':
          $sorted_legislators[6][$legislator->role][] = $legislator;
          break;
      }
    }
  }

  // Order the result by numeric array key.
  ksort($sorted_legislators);

  // Remove the numeric array keys and replace with legislative role.
  $final_sort = array();
  foreach ($sorted_legislators as $index => $members) {
    $final_sort = array_merge($final_sort, $members);
  }

  return $final_sort;
}


/**
 * Returns the legislator roles.
 */
function sba_leg_lookup_legislator_roles() {
  return array(
    'FS' => t('Federal Senator'),
    'FR' => t('Federal Representative'),
    'SS' => t('State Senator'),
    'SR' => t('State Representative'),
    'GOVNR' => t('Governor'),
    'PRES01' => t('President'),
    'PRES03' => t('Vice President'),
  );
}

function sba_leg_lookup_states() {
  $states = array(
    '' => '',
    'AL' => 'Alabama',
    'AK' => 'Alaska',
    'AZ' => 'Arizona',
    'AR' => 'Arkansas',
    'CA' => 'California',
    'CO' => 'Colorado',
    'CT' => 'Connecticut',
    'DE' => 'Delaware',
    'DC' => 'District Of Columbia',
    'FL' => 'Florida',
    'GA' => 'Georgia',
    'HI' => 'Hawaii',
    'ID' => 'Idaho',
    'IL' => 'Illinois',
    'IN' => 'Indiana',
    'IA' => 'Iowa',
    'KS' => 'Kansas',
    'KY' => 'Kentucky',
    'LA' => 'Louisiana',
    'ME' => 'Maine',
    'MD' => 'Maryland',
    'MA' => 'Massachusetts',
    'MI' => 'Michigan',
    'MN' => 'Minnesota',
    'MS' => 'Mississippi',
    'MO' => 'Missouri',
    'MT' => 'Montana',
    'NE' => 'Nebraska',
    'NV' => 'Nevada',
    'NH' => 'New Hampshire',
    'NJ' => 'New Jersey',
    'NM' => 'New Mexico',
    'NY' => 'New York',
    'NC' => 'North Carolina',
    'ND' => 'North Dakota',
    'OH' => 'Ohio',
    'OK' => 'Oklahoma',
    'OR' => 'Oregon',
    'PA' => 'Pennsylvania',
    'RI' => 'Rhode Island',
    'SC' => 'South Carolina',
    'SD' => 'South Dakota',
    'TN' => 'Tennessee',
    'TX' => 'Texas',
    'UT' => 'Utah',
    'VT' => 'Vermont',
    'VA' => 'Virginia',
    'WA' => 'Washington',
    'WV' => 'West Virginia',
    'WI' => 'Wisconsin',
    'WY' => 'Wyoming',
  );

  return $states;
}
