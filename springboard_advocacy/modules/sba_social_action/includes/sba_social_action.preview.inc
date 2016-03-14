<?php

/**
 * Social action preview page, menu callback.
 *
 * @return array
 */
function sba_social_action_message_preview_page($nid) {

  // $nid already sanitized in the access hook.
  $node = node_load($nid);
  //$test_mode = field_get_items('node', $node, 'field_sba_test_mode');

  // Build the form content out of the API response stored in session.
  // First sort the messages.
  $messages = array();
  $random_array = array();

  if (!empty($_SESSION['deliverable_social_messages']['messages'])) {
    foreach ($_SESSION['deliverable_social_messages']['messages'] as $id => $message) {

      // Create a randomized array of message ids for random display of alternate tweet texts in the form
      // Placed here to avoid complications from ajax reload after twitter authentication.
      if(!empty($node->messages[$message['message_id']]->field_sba_twitter_message[LANGUAGE_NONE])) {
        $mess = count($node->messages[$message['message_id']]->field_sba_twitter_message[LANGUAGE_NONE]);
      }
      else {
        $mess = 0;
      }

      for ($x=0; $x<$mess; $x++) {
        $random_array[$node->messages[$message['message_id']]->sba_message_id][] = mt_rand(0, $mess - 1);
      }

      // Sort criteria for message targets.
      $messages[$node->messages[$message['message_id']]->data['weight'] . $message['person']['organization'] . $message['targets']->last_name] = $message;
      unset($messages[$id]);
    }
  }

  if (!empty($_SESSION['undeliverable_social_messages']['messages'])) {
    foreach ($_SESSION['undeliverable_social_messages']['messages'] as $message) {
      // Sort criteria for message targets.
      $messages[$node->messages[$message['message_id']]->data['weight'] . $message['person']['organization'] . $message['targets']->last_name] = $message;
    }
  }
  ksort($messages);

  if(!empty($node->field_sba_test_mode[0]['value'])) {
    drupal_set_message('This action is in test mode. Messages will not be not sent.', 'warning', TRUE);
  }

  if (isset($_SESSION['messages']['status'])) {
    // Prevent the confirmation message from appearing prematurely.
    $_SESSION['saved_messages']['status'] = $_SESSION['messages']['status'];
    // clear all
    drupal_get_messages('status');
  }

  // Check if the step two form has already been submitted.
  $completed = isset($_SESSION['social_action_sid']['completed']) ? $_SESSION['social_action_sid']['completed'] : TRUE;
  if($completed == TRUE) {
    drupal_set_message(t("This form has already been submitted. !return to the form.", array('!return' => l('Return', 'node/' . $node->nid))));
    return array();
  }

  // Build page header and multi-step form.
  $build = array();
  $header_field = field_get_items('node', $node, 'field_sba_social_step_two_header');
  $header_value = field_view_value('node', $node, 'field_sba_social_step_two_header', $header_field[0]);
  $markup = isset($header_value['#markup']) ? $header_value['#markup'] : '';
  $build['content'] = array(
    'title'  => array(
      '#prefix' => '<h2>',
      '#suffix' => '</h2>',
      '#weight' => -11,
      '#markup' => check_plain($node->title),
    ),
    'step' => array(
        '#prefix' => '<h4>',
        '#suffix' => '</h4>',
        '#weight' => -10,
        '#markup' => t('Step 2 of 2'),
      ),
    'header' => array(
    '#weight' => -9,
    '#markup' => $markup,
  ),
    'noscript' => array('#markup' => '<noscript><div class="alert error">Javascript is required. Please enable javascript in your browser.</div></noscript>'),
    'sba_social_action_preview_form' => drupal_get_form('sba_social_action_preview_form', $node, $messages, $random_array),
  );

  return $build;
}

/**
 * Multi-step form definition.
 *
 * This form is mostly within a container that gets ajax replaced when the Oauth
 * login window popup is closed, which causes the form elements to
 * become non-disabled.
 *
 * @param $form
 * @param $form_state
 * @param $node
 *
 * @return mixed
 */
function sba_social_action_preview_form($form, $form_state, $node, $messages, $random_array) {
  // Access variable to determine form element accessibility.
  $access = FALSE;
  module_load_include('inc', 'sba_social_action', 'includes/sba_social_action.oauth_cache');
  $twitter_user_cache = sba_social_action_cache_get('sba_twitter');
  $form['twitter_user_cache'] = array (
    '#type' => 'value',
    '#value' => $twitter_user_cache,
  );
  if (!empty($twitter_user_cache)) {
    $access = TRUE;
    $twitter_user_cache['values']['oauth_token'] = decrypt($twitter_user_cache['values']['oauth_token']);
    $twitter_user_cache['values']['oauth_token_secret'] = decrypt($twitter_user_cache['values']['oauth_token_secret']);
  }

  // Bypass authentication and use Twitter web intents if that node option is set.
  if (empty($node->field_sba_social_require_auth)) {
    $access = TRUE;
  }

  $path = drupal_get_path('module', 'sba_social_action');
  $form['#attached']['css'][] = $path . '/css/message-preview.css';
  $form['#attached']['js'][] = $path . '/js/jQuery.simplyCountable.js';
  $form['#attached']['js'][] = $path . '/js/social_counter.js';
  $form['#attached']['js'][] = $path . '/js/social_message_preview.js';
  $form['#attached']['js'][] = $path . '/js/jquery.oauthpopup.js';

  // Invisible form element used to trigger ajax update when popup widow closes.
  $form['ajaxify'] = array(
    '#type' => 'checkbox',
    '#attributes' => array('style' => array('display:none')),
    '#title' => '',
    '#ajax' => array(
      'callback' => 'sba_social_actions_reload_form',
      'wrapper' => 'form-container',
      'effect' => 'fade',
      'speed' => 1000,
      'progress' => 'none',
      'event' => 'mousedown'
    ),
  );

  $num_mess = count($messages);
  $count = 0;

  // Ajax container
  $form['form_container'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="form-container">',
    '#suffix' => '</div>',
  );

  // Sign in with Twitter.
  if (empty($twitter_user_cache)  && $access == FALSE) {
    $url = variable_get('sba_social_privacy', '');
    if (!empty($url)) {
      $url = l('Privacy Policy', $url, array('attributes' => array('target' => '_new', 'class' => 'sba-privacy')));
    }
    $form['form_container']['twitter_sign_in'] = array(
      '#type' => 'button',
      '#value' => 'Authorize with Twitter',
      '#attributes' => array('class' => array('twitter-sign')),
      '#prefix' => '<div class="twit-auth">',
      '#suffix' => $url . '</div>',
    );
  }

  // @todo explore other ways of retrieving these two.
  $contact = $_SESSION['social_contact'];
  $action = $_SESSION['social_form_details'];
  // A couple values we'll need later
  $form['action_id'] = array(
    '#type' => 'value',
    '#value' => $action['action_id'],
  );
  $form['contact'] = array(
    '#type' => 'value',
    '#value' => $contact,
  );

  // Rebuild the messages for display
  $form['form_container']['messages'] = array (
    '#type' => 'container',
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('message-preview-container'),
    ),
  );
  // Used to send our second step form through the same helper functions as the first step form.
  module_load_include('inc', 'sba_social_action', 'includes/sba_social_action.form');
  static $id = 0;
  static $n = 0;
  foreach ($messages as $key => $message) {

    // What type of message is this.
    $message_entity = $node->messages[$message['message_id']];
    $message_entity_type = $node->messages[$message['message_id']]->type;
    $editable = field_get_items('sba_message', $node->messages[$message['message_id']], 'field_sba_twitter_editable');
    $editable = $editable[0]['value'] != 'Not editable' ? TRUE : FALSE;
    $form['form_container']['messages'][$key] = array(
    '#prefix' => '<div class="message-preview-message-container">',
      '#suffix' => '</div>',
    );
    // Theme the message header.
    $form['form_container']['messages'][$key]['sent_to'] = array(
      '#markup' => theme('sba_social_action_preview_message_header', array('message' => $message, 'type' => $message_entity_type, 'editable' => $editable)),
    );

    // Create the message form elements from the entity. Passed by reference
    // to the same helper function as the first step form.
    $build = array();
    sba_social_action_message_form($build, $message_entity, $message_entity_type, $message_entity_type);

    $this_message =& $build['message'][$message_entity->sba_message_id];
    $this_message['#attributes']['class'][] = 'message-preview-message-fieldset';
    if ($count == 0) {
      $this_message['#attributes']['class'][] = 'first';
    }
    if ($count == $num_mess - 1) {
      $this_message['#attributes']['class'][] = 'last';
    }
    $count++;

    $prepend = !empty($this_message['prepend']['#value']) ? '.' : '';
    $send_tweet ='';
    if (isset($this_message['body_show'])) {
      $message_alt = field_get_items('sba_message', $message_entity, 'field_sba_twitter_message');
      $alternatives = array();
      if (is_array($message_alt)) {
        foreach ($message_alt as $alt) {
          $alt_tweet = $prepend . '@' . $message['person']['twitter'] . ' ' . $alt['value'];
          $alternatives[] = $alt_tweet;
        }
      }

      // Reset $n if there are multiple messages.
      if ($id != $message_entity->sba_message_id) {
        $n = 0;
      }
      // $random_array is defined in the build function so that it does not
      // change after the form ajax reload.
      if (isset($random_array[$message_entity->sba_message_id])) {
        $rand_count = count($random_array[$message_entity->sba_message_id]);
        $rand_mess = $random_array[$message_entity->sba_message_id];
        $tweet = $alternatives[$rand_mess[$n]];
      }
      else {
        $tweet = current($alternatives);
      }
      $display_tweet = nl2br(check_plain($tweet));
      $send_tweet = $tweet;
      $this_message['body_show']['#markup'] = $display_tweet;
      if (isset($rand_count) && $n < ($rand_count - 1)) {
        $n++;
      }
      else {
        $n = 0;
      }
      $id = $message_entity->sba_message_id;
    }


    if (isset($this_message['edited_body']) && $message_entity_type == 'sba_social_action_twitter_message') {
      $prepend = !empty($this_message['prepend']['#value']) ? '.' : '';
      $this_message['body_show']['#suffix'] = '<div class="sba-full-tweet"><div class="sba-display-hint">This will be your full tweet:</div><div class="editable-message-preview"><span class="preview-target-text">' . $prepend . '@' . $message['person']['twitter'] . '</span><span class="preview-replace-text"></span></div></div>';
    }

    if (!isset($this_message['edited_body']) && $access == TRUE && $message_entity_type == 'sba_social_action_twitter_message') {
      $text = $send_tweet;
      $link = l('Customize and send this tweet', 'https://twitter.com/intent/tweet', array('query' => array('text' => $text)));
      $img = drupal_get_path('module', 'sba_social_action') . '/images/Twitter_logo_blue_32.png';
      $this_message['body_show']['#suffix'] = '<div class="twitter-asset"><img src="/' . $img . '"><div class="sba-web-intent-link">' . $link . '</div></div>';
    }

    // Build an array of target properties
    $this_message['target'] = array(
      '#type' => 'value',
      '#value' => array(
        'target_id' => $message['targets']->id,
        'twitter' => $message['targets']->twitter,
        'salutation' => $message['targets']->salutation,
        'first_name' => $message['targets']->first_name,
        'last_name' => $message['targets']->last_name
      ),
    );

    // Check for required fields
    if (isset($this_message['body_required']) && $this_message['body_required']['#value']) {
      $this_message['edited_body']['#required'] = TRUE;
      $this_message['edited_body']['#title'] =  t('You must edit the body of this message before sending');
    }
    if(isset($this_message['edited_body'])) {
      $this_message['edited_body']['#disabled'] = !$access;
    }

    if (isset($this_message['subject_required']) && $this_message['subject_required']['#value']) {
      $this_message['subject']['#required'] = TRUE;
      $this_message['subject']['#title'] =  t('Subject - You must edit this subject line before sending');
    }

    // Build the alternative social media links
    $alternates = $this_message['alternate_options']['#value'];
    $alt_links = array();
    if (!empty($alternates)) {
      foreach ($alternates as $alternate) {
        if (!empty($message['targets']->{strtolower($alternate)})) {
          $web = $alternate == 'Website' ? t('Official website') : $alternate;
          $alt_links[] =  l($web, $message['targets']->{strtolower($alternate)}, array('attributes' => array('target' => '_new')));
        }
      }
    }
    if (!empty($alt_links)) {
      $this_message['alternate'] = array(
        '#markup' => t('If you prefer, please contact @target via: !methods', array('@target' => $message['person']['name'], '!methods' => '<br />' . implode(' <br /> ', $alt_links))),
        '#prefix' => '<div class="sba-alternate-contact">',
        '#suffix' => '</div>',
      );
    }
    // Assign the message to the messages container.
    $form['form_container']['messages'][$key]['message'] = $this_message;
  }

  // Following are some header area content which need to be here in the code
  //  to get ajax updated values.
  if($access == TRUE) {
    $markup = '';
    // Commented out for web intents version.
    //$markup = ('Please review the messages below; or if you prefer, simply <span id="multiflow-send-now-wrapper" style="display:inline-block"><a href="#" id="multiflow-send-now">send now</a></span>.');
  }
  else {
    $markup = t('In order to post, you must authorize our web app to access your Twitter account.');
  }
  $messages = $form['form_container']['messages'];
  $children = array_flip(element_children($messages));
  $subject_required = FALSE;
  $body_required = FALSE;
  $editable_but_not_required = FALSE;
  foreach ($messages as $key => $message) {
    if(isset($children[$key]) && (!$subject_required && !$body_required)) {
      if(isset( $message['subject']) && !isset($message['subject_show'])) {
        // Not used for Twitter but will be used for Facebook?
        $editable_but_not_required = TRUE;
        $subject_required = isset($message['message']['subject']['#required']) ? $message['message']['subject']['#required'] : FALSE;
      }
      if (isset($message['message']['edited_body'])) {
        $editable_but_not_required = TRUE;
        $body_required = $message['message']['edited_body']['#required'];
      }
    }
  }

  // Shown after the ajax reload from authentication.
  if (isset($twitter_user_cache['values']['screen_name']) && !empty($node->field_sba_social_require_auth)) {
    $form['form_container']['page_description_login'] = array(
      '#prefix' => '<div class="preview-twitter-user"><p>',
      '#suffix' => '</p></div>',
      '#weight' => -9,
      '#markup' => t('You will be posting to Twitter as !name. If that is not the correct account, !clickhere.', array(
        '!name' => '<span class="preview-name">@' . $twitter_user_cache['values']['screen_name'] . '</span>',
        '!clickhere' => l('click here', 'clear/session', array('query' => array('destination' => 'node/' . $node->nid . '/social-message-preview')))
      )),
    );
  }

  // Not needed for twitter web intents for perhaps for facebook?
  if ($subject_required || $body_required) {
    $editable_but_not_required = FALSE;
    $form['form_container']['page_description_content']['#markup'] = t('One or more of the messages below requires editing. Please make your edits and then click "Send Now" at the bottom of the page.');
  }
  if (!empty($editable_but_not_required) && $access == TRUE) {
    $markup = t('Please review and edit the messages below.');
  }
  $form['form_container']['page_description_content'] = array(
    '#id' => 'social-action-preview-page',
    '#type' => 'markup',
    '#markup' => $markup,
    '#prefix' => '<div class="social-action-preview-page">',
    '#suffix' => '</div>',
    '#weight' => -8,
  );
  // End header area content

  // Add the node object to use in validation and submit.
  $form['#action_node'] = $node;
  // Define the validation function.
  //$form['#validate'][] = 'sba_social_action_preview_form_validate';
  $form['#submit'][] = 'sba_social_action_preview_form_submit';

  $submit_field = field_get_items('node', $node, 'field_sba_social_step_two_submit');
  $markup = !empty($submit_field[0]['value']) ? $submit_field[0]['value'] : t("I'm done");
  // Add submit button
  $form['form_container']['submit'] = array(
    '#prefix' => '<div id="edit-actions">',
    '#suffix' => '</div>',
    '#type' => 'submit',
    '#value' => $markup,
    '#weight' => 10,
    '#attributes' => array('class' => 'social-action-preview-submit'),
   // '#disabled' => !$access,
  );
  return $form;
}

/**
 * Ajax callback.
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function sba_social_actions_reload_form($form, $form_state) {
  // Save the Twitter user object after authorization.
  $sid = !empty($_SESSION['social_action_sid']['sid']) ? $_SESSION['social_action_sid']['sid'] : FALSE;
  if ($sid) {
    module_load_include('inc', 'webform', 'includes/webform.submissions');
    $submission = webform_get_submission($form['#action_node']->nid, $sid, TRUE);
    module_load_include('inc', 'sba_social_action', 'includes/sba_social_action.oauth_cache');
    $twitter_user_cache = sba_social_action_cache_get('sba_twitter');
    if (!empty($twitter_user_cache) && !empty($submission->uid)) {
      sba_social_action_save_twitter_profile($twitter_user_cache, $submission->uid);
    }
  }

  // Ajax reload form with auth username data.
  return $form['form_container'];
}

/**
 * Validate the  messages.
 *
 * The web intents version does not need this function. Keeping for future use.
 * if Twitter posting API passes muster.
 *
 * @param $form
 * @param $form_state
 */
//function sba_social_action_preview_form_validate($form, &$form_state) {
//  $messages = !empty($form['messages']) ? element_children($form['messages']) : array();
//  $changed = FALSE;
//  // To prevent duplicate error messages.
//  $body_error_once = NULL;
//  $subject_error_once = NULL;
//
//  // Loop through the messages and validate whether user is required to edit subject or body.
//  // Also set a value for future use by the "user_edit_flag" component.
//  foreach ($messages as $id) {
//
//    // The message
//    $message = $form['messages'][$id]['message'];
//
//    // The default value for the user-editable body if any.
//    $body_default = isset($message['edited_body']['#default_value']) ? $message['edited_body']['#default_value'] : '';
//    // The edited value in the user-editable body if any.
//    $body_edited = isset($form_state['values']['messages'][$id]['message']['edited_body']) ? $form_state['values']['messages'][$id]['message']['edited_body'] : '';
//
//    // The default value for the user-editable subject if any.
//    $subject_default = isset($message['subject']['#default_value']) ? $message['subject']['#default_value'] : '';
//    // The edited value in the user-editable subject if any.
//    $subject_edited = isset($form_state['values']['messages'][$id]['message']['subject']) ? $form_state['values']['messages'][$id]['message']['subject'] : '';
//
//    // Enforce mandatory editing of the body if set.
//    $body_error = !isset($body_error_once) ? t('Please personalize your message before sending.') : '';
//    if ($body_edited == $body_default && isset($message['body_required']) && $message['body_required']['#value'] == 1) {
//      $body_error_once = TRUE;
//      form_set_error("messages][$id][message][edited_body", $body_error);
//    }
//    elseif ($body_edited != $body_default) {
//      // Set change value for use by the "user_edit_flag" component.
//      $changed = TRUE;
//    }
//
//    // Enforce mandatory editing of the subject if set.
//    $subject_error = !isset($subject_error_once) ? t('Please personalize the subject line before sending your message.') : '';
//    if ($subject_edited == $subject_default && isset($message['subject_required']) && $message['subject_required']['#value'] == 1) {
//      $subject_error_once = TRUE;
//      form_set_error("messages][$id][message][subject", $subject_error);
//    }
//    elseif (!isset($message['subject_show']) && $subject_edited != $subject_default) {
//      // Set change value for use by the "user_edit_flag" component.
//      $changed = TRUE;
//    }
//  }
//  // Set value for the user_edited_flag
//  $form_state['storage']['changed'] = $changed;
//
//  // Update error messages.
//  if (!empty($_SESSION['messages']['error'])) {
//    foreach ($_SESSION['messages']['error'] as $key => $message) {
//      if (strpos($message, 'You must edit the body of this message before sending') !== FALSE) {
//        unset($_SESSION['messages']['error'][$key]);
//        drupal_set_message('Please personalize your message before sending', 'error', FALSE);
//      }
//      if (strpos($message, 'You must edit the subject of this message before sending') !== FALSE) {
//        unset($_SESSION['messages']['error'][$key]);
//        drupal_set_message('Please personalize the subject line before sending', 'error', FALSE);
//      }
//    }
//  }
//}

/**
 * Submit the messages to the API endpoint.
 *
 * @param $form
 * @param $form_state
 */
function sba_social_action_preview_form_submit($form, &$form_state) {
  module_load_include('php', 'oauth_common', 'lib/OAuth');
  module_load_include('php', 'sba_social_action', 'includes/socialTwitter.lib');
  $node = $form['#action_node'];
  $sid = !empty($_SESSION['social_action_sid']['sid']) ? $_SESSION['social_action_sid']['sid'] : FALSE;
  if ($sid) {
    module_load_include('inc', 'webform', 'includes/webform.submissions');
    $submission = webform_get_submission($node->nid, $sid);
  }

  // Web intents version does not use this. Keeping for future use.
//  foreach ($form_state['values']['messages'] as $message) {

//    $message_text = isset($message['message']['edited_body']) ? $message['message']['edited_body'] : $message['message']['body'];
//    $target = parse_url($message['message']['target']['twitter']);
//    $prepend = !empty($message['message']['prepend']) ? '.' : '';
//    $body = $prepend . '@' . substr($target['path'], 1) . ' ' . $message_text;
//
//    if(empty($node->field_sba_test_mode)) {
//      try {
//        $twitter_user_cache = $form_state['values']['twitter_user_cache'];
//        if(!empty($twitter_user_cache)) {
//          $twitter_user_cache['values']['oauth_token'] = decrypt($twitter_user_cache['values']['oauth_token']);
//          $twitter_user_cache['values']['oauth_token_secret'] = decrypt($twitter_user_cache['values']['oauth_token_secret']);
//
//          $twitter = new socialTwitter(
//            variable_get('social_action_twitter_consumer_key', ''),
//            variable_get('social_action_twitter_consumer_secret', ''),
//            $twitter_user_cache['values']['oauth_token'],
//            $twitter_user_cache['values']['oauth_token_secret']
//          );
//
//          $response = $twitter->statuses_update($body);
//
//          if (!empty($response['id'] && !empty($submission))) {
//            $twitter_status = new SbaTwitterStatus();
//            $twitter_status->twitter_status_id = $response['id'];
//            $twitter_status->screen_name = $response['user']['screen_name'];
//            $twitter_status->created = strtotime($response['created_at']);
//            $twitter_status->retweet_count = $response['retweet_count'];
//            $twitter_status->favorite_count = $response['favorite_count'];
//            $twitter_status->uid = $submission->uid;
//            $twitter_status->nid = $node->nid;
//            $twitter_status->sid = $sid;
//            $twitter_status->message_entity_id = $message['message']['sba_message_id'];
//            $twitter_status->target_id = $message['message']['target']['target_id'];
//            $twitter_status->entities = '';//$response['entities'];
//            $twitter_status->is_new = TRUE;
//            entity_save('sba_twitter_status', $twitter_status);
//          }
//        }
//
//      } catch (socialTwitterException $e) {
//        drupal_set_message(t('An error occurred when posting to Twitter. Check the system log for details.'), 'error', TRUE);
//      }
//    }
//    else {
//      //drupal_set_message('This action was in test mode. Messages were not sent.', 'warning', TRUE);
//    }
//  }


  // Redirect to the original confirmation page.
  // $sid is set in sba_social_action_multifow_submit()
  $_SESSION['social_action_sid']['completed'] = TRUE;
  springboard_advocacy_negotiate_redirect($node, $sid, $form_state);
  $changed = !empty($form_state['storage']['changed']) ? $form_state['storage']['changed'] : FALSE;
  if ($sid) {
    springboard_advocacy_update_multistep_components($sid, $node, $changed);
  }
}

/**
 * Theme the message preview header, i.e., recipient name
 *
 * @param $vars
 * @return string
 */
function theme_sba_social_action_preview_message_header($vars) {
  $output = '<div class="message-preview-header"><div class="message-preview-intro">';
  $output .= '</div><div class="message-preview-person">';
  $output .= t('To') . ' ' . theme('sba_social_action_person_details', $vars);
  $output .= '</div></div>';
  return $output;
}
