<?php
// $Id$

function _feedburner_build_overview() {
  // Add the filter form above the overview table.
  $keys = _feedburner_path_keys();
  $output = drupal_get_form('_feedburner_build_filter_form', $keys);

  if (!empty($keys)) {
    // Replace wildcards with MySQL/PostgreSQL wildcards.
    $keys = preg_replace('!\*+!', '%', $keys);
    $sql = "SELECT f.local_uri, f.fb_uri, f.verified FROM {feedburner_feeds} f LEFT JOIN {url_alias} u ON f.local_uri = u.src WHERE f.local_uri LIKE '%%%s%%' OR f.fb_uri LIKE '%%%s%%' OR u.dst LIKE '%%%s%%'";
  }
  else {
    $sql = 'SELECT f.local_uri, f.fb_uri, f.verified FROM {feedburner_feeds} f';
  }
  $header = array(
    array('data' => t('System Path'), 'field' => 'f.local_uri', 'sort' => 'asc'),
    array('data' => t('FeedBurner'), 'field' => 'f.fb_uri'),
    array('data' => t('Verified'), 'field' => 'f.verified'),
    array('data' => t('Operations'), 'colspan' => '3'),
  );
  $sql .= tablesort_sql($header);
  $result = pager_query($sql, 50, 0, null, $keys, $keys, $keys);

  $rows = array();
  //$destination = drupal_get_destination();
  while ($data = db_fetch_object($result)) {
    $alias = drupal_get_path_alias($data->local_uri);
    $rows[] = array(
      l($data->local_uri . ($data->local_uri != $alias ? ' ('. $alias .')' : ''), $data->local_uri),
      l($data->fb_uri, _feedburner_url($data->fb_uri)),
      ($data->verified > 0 ? 'Yes' : l(t('No'), 'feedburner/verify/'. $data->fb_uri, null, 'local_uri='. $data->local_uri)),
      l(t('edit'), 'admin/build/feedburner/edit/'. $data->local_uri),
      l(t('remove'), 'admin/build/feedburner/remove/'. $data->local_uri),
      l(t('source'), $data->local_uri, array(), 'FEEDBURNER=NO_REDIRECT'),
    );
  }

  if (empty($rows)) {
    if (empty($keys)) {
      $alias = drupal_get_path_alias('rss.xml');
      $rows[] = array(
        l('rss.xml'. ('rss.xml' != $alias ? ' ('. $alias .')' : ''), 'rss.xml'),
        'None', '', l(t('redirect'), 'admin/build/feedburner/add/rss.xml'),
      );
    }
    else {
      $rows[] = array(array('data' => t('No FeedBurner feeds found.'), 'colspan' => '5'));
    }
  }

  $output .= theme('table', $header, $rows);
  $output .= theme('pager', null, 50, 0);

  $output .= feedburner_help('feedburner_about');

  return $output;
}

/**
 * Return a form to filter FeedBurner feeds.
 */
function _feedburner_build_filter_form($keys = '') {
  $form['#attributes'] = array(
    'class' => 'search-form',
  );
  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter feeds'),
  );
  $form['basic']['inline'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );
  $form['basic']['inline']['filter'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => $keys,
    '#maxlength' => 64,
    '#size' => 25,
  );
  $form['basic']['inline']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );

  return $form;
}

/**
 * Process filter form submission.
 */
function _feedburner_build_filter_form_submit($form_id, $form_values) {
  return 'admin/build/feedburner/list/'. trim($form_values['filter']);
}

function _feedburner_build_edit_form($edit = '') {
  $form['#base'] = '_feedburner_build_edit_form';

  $form['local_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Local Feed'),
    '#default_value' => $edit['local_uri'],
    '#maxlength' => 128,
    '#size' => 30,
    //'#disabled' => !empty($local_uri),
    '#description' => t('Specify the existing feed on your site from which to redirect.'),
    '#field_prefix' => url(null, null, null, true) . (variable_get('clean_url', 0) ? '' : '?q='),
    '#validate' => array('_feedburner_build_edit_validate' => array('local_uri')),
    '#autocomplete_path' => 'feedburner/autocomplete',
  );
  $form['fb_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('FeedBurner Feed'),
    '#default_value' => $edit['fb_uri'],
    '#maxlength' => 100,
    '#size' => 30,
    '#description' => t('Requests to the above feed will be requested to this FeedBurner feed. This URI is case-sensitive and alphanumeric.'),
    '#field_prefix' => _feedburner_url(),
    '#validate' => array('_feedburner_build_edit_validate' => array('fb_uri')),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save redirection'),
  );

  return $form;
}

function _feedburner_build_edit_validate($form_element, $field_name) {
  $value = $form_element['#value'];
  switch ($field_name) {
    case 'local_uri':
      $value = strtolower(drupal_get_normal_path($value));
      form_set_value($form_element, $value);
      /*$data = drupal_http_request(url($value, null, null, true));
      if ($data->code != 200) {
        drupal_set_message(t('The local feed %path could not be validated.'), 'error')
      }*/
      break;
    case 'fb_uri':
      if (!preg_match('/\\A[\\w\\x2D\\x2F]*\\z/i', $value)) {
        form_set_error($field_name, 'Invalid FeedBurner URI');
      }
      break;
  }
}

function _feedburner_build_edit_form_submit($form_id, $form_values) {
  if (_feedburner_save($form_values['local_uri'], $form_values['fb_uri']) == true) {
    drupal_set_message(t('The FeedBurner redirect has been saved.'));
    _feedburner_verify_feed($form_values['fb_uri'], $form_values['local_uri']);
    //return 'feedburner/verify/'. $form_values['fb_uri'];
  }
  return 'admin/build/feedburner';
}

function _feedburner_build_remove() {
  $path = _feedburner_path_keys();
  if (!empty($path) && _feedburner_save($path) == true) {
    drupal_set_message(t('The FeedBurner redirection for %path has been removed.', array('%path' => $path)));
  }
  drupal_goto('admin/build/feedburner');
}

function _feedburner_settings_form() {
  $form['redirection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feed Redirection'),
  );
  $form['redirection']['feedburner_redirect_status'] = array(
    '#type' => 'select',
    '#title' => t('Redirect Header Status'),
    '#default_value' => variable_get('feedburner_redirect_status', FEEDBURNER_DEFAULT_REDIRECT_STATUS),
    '#options' => array(
      301 => '301 Moved Permanently',
      302 => '302 Found',
      307 => '307 Temporary Redirect'),
    '#description' => t("Please note that the '307 Temporary Redirect' is recommended but if your subscribers are having problems you may want to try a different status code. For detailed descriptions of HTTP redirect status codes, see <a href=\"@link\">the Wikipedia article on HTTP status codes</a>.", array('@link' => 'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection')),
  );
  $form['redirection']['feedburner_blogs'] = array(
    '#type' => 'checkbox',
    '#title' => t("Allow users to specify their own blogs' FeedBurner feed."),
    '#default_value' => variable_get('feedburner_blogs', false),
    '#description' => t("If the user has the permission 'edit own blog', this option is found in his/her 'edit account' page."),
  );

  $form['feedflare'] = array(
    '#type' => 'fieldset',
    '#title' => t('FeedFlare/Standard Stats/Ad Network'),
    '#description' => t('See <a href="@link">@link</a> for more information.', array('@link' => 'http://www.feedburner.com/fb/a/publishers/feedflare')),
    '#collapsible' => false,
  );
  /*if (!array_key_exists('rss2.xml', _feedburner_get_redirects())) {
    $form['feedflare']['notice'] = array(
      '#value' => '<p><strong>FeedFlare is only enabled when you have a FeedBurner feed for your site\'s main feed (rss.xml).</strong></p>',
    );
  }*/
  $form['feedflare']['feedburner_feedflare_node_types'] = array(
    //'#type' => 'checkboxes',
    '#type' => 'select',
    '#title' => t('Node types'),
    '#multiple' => true,
    '#default_value' => variable_get('feedburner_feedflare_node_types', array()),
    '#options' => node_get_types('names'),
  );
  $form['feedflare']['feedburner_feedflare'] = array(
    '#type' => 'select',
    '#title' => t('FeedFlare in nodes'),
    '#default_value' => variable_get('feedburner_feedflare', 0),
    //'#disabled' => !array_key_exists('rss.xml', _feedburner_get_redirects()),
    '#options' => array(
      0 => t('Disabled'),
      1 => t('Teaser view'),
      2 => t('Full-page view'),
      3 => t('Teasers and full-page view')),
    '#description' => t('When to display the FeedFlare after the node text.'),
  );

  /*$form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('FeedBurner Account'),
    '#description' => t('Currently your username and password are stored as plaintext in the Drupal database. FeedBurner has yet to allow more secure methods of authentication.'),
    '#collapsible' => true,
    '#collapsed' => true,
  );
  $form['account']['feedburner_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Your FeedBurner username'),
    '#default_value' => variable_get('feedburner_username', ''),
    '#size' => 30,
    '#maxlength' => 30,
  );*/
  /*$form['account']['feedburner_password'] = array(
    '#type' => 'password',
    '#title' => t('Your FeedBurner account password'),
    '#default_value' => variable_get('feedburner_password', ''),
    '#size' => 30,
    '#maxlength' => 30,
  );*/

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Settings'),
    '#collapsible' => true,
    '#collapsed' => true,
  );
  $form['advanced']['feedburner_useragents'] = array(
    '#type' => 'textarea',
    '#title' => t('User Agents Allowed Direct Feed Access'),
    '#description' => t('Enter a list of user agents (one per line) that are allowed direct access to your site\'s Feeds and will not be redirected to the FeedBurner feeds.'),
    '#default_value' => preg_replace('/\\|/', "\n", variable_get('feedburner_useragents', FEEDBURNER_DEFAULT_USERAGENTS)),
    '#rows' => 3,
    '#validate' => array('_feedburner_settings_validate' => array('feedburner_useragents')),
  );
  $form['advanced']['feedburner_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('MyBrand Custom Domain'),
    '#description' => t('<strong>For Advanced Users Only!</strong> MyBrand is a premium FeedBurner service that allows you to showcase your feeds by serving them from your own domain (e.g., @domain). You must have access to change your CNAME entry in the DNS records for the domain you wish to change. For more information, see <a href="@link">@link</a>.', array('@link' => 'http://www.feedburner.com/fb/a/mybrand', '@domain' => 'feeds.'. $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('feedburner_domain', FEEDBURNER_DEFAULT_DOMAIN),
    '#field_prefix' => 'http://',
    '#field_suffix' => '/feedname',
    '#size' => 30,
    '#maxlength' => 30,
    '#validate' => array('_feedburner_settings_validate' => array('feedburner_domain')),
  );

  $form['blah'] = array(
    '#type' => 'markup',
    '#prefix' => '',
    '#suffix' => '',
    '#value' => feedburner_help('feedburner_about'),
    '#weight' => 10,
  );

  return system_settings_form($form);
}

/**
 * Validate and correct values from the settings form.
 * - Format user agents from a list to a regex-ready string
 * - Check custom domain name syntax
 */
function _feedburner_settings_validate($form_element, $field_name) {
  $value = $form_element['#value'];
  switch ($field_name) {
    case 'feedburner_useragents':
      // Replace one or more consecutive line breaks or pipes with regex pipes,
      // remove any non-word non-pipe characters and also remove any pipes at
      // the beginning or end of the string
      $value = preg_replace(array('/[\\r\\n\\|]+/', '/[^\\|\\w]|\\A\\|+|\\|+\\z/'), array('|', ''), $value);
      form_set_value($form_element, $value);
      break;
    case 'feedburner_domain':
      if ($value != FEEDBURNER_DEFAULT_DOMAIN) {
        $data = drupal_http_request('http://'. $value);
        if (!isset($data->redirect_url) && ($data->redirect_url != 'http://www.feedburner.com/fb/a/home' || preg_match('/\/fb\/a\/home$/', $data->redirect_url))) {
          drupal_set_message($field_name, 'The MyBrand domain could not be confirmed. Make sure to edit the CNAME settings for the domain before enabling this option.', 'error');
        }
      }
      break;
  }
}

/*
 * Extract the remaining part of the query url from a certain argument onward.
 * @param pos the position in the query argument, default 5th
 * @return the remaining url paramenters (including any backslashes)
 */
function _feedburner_path_keys($pos = 5) {
  $path = explode('/', $_GET['q'], $pos);
  return count($path) == $pos ? $path[$pos - 1] : '';
}

/**
 * Updates or removes a feed in the database.
 * @param local_uri The local system feed uri to update.
 * @param fb_uri The FeedBurner feed uri (set to null to remove redirection).
 * @return true if the redirection is updated or removed, false otherwise.
 */
function _feedburner_save($local_uri, $fb_uri = null) {
  //$result = _feedburner_load($local_uri);
  if (empty($fb_uri)) {
    db_query("DELETE FROM {feedburner_feeds} WHERE local_uri = '%s'", $local_uri);
  }
  else {
    db_query("UPDATE {feedburner_feeds} SET fb_uri = '%s' WHERE local_uri = '%s'", $fb_uri, $local_uri);
    $rows = db_affected_rows();
    if (empty($rows)) {
      db_query("INSERT INTO {feedburner_feeds} (local_uri, fb_uri) VALUES ('%s', '%s')", $local_uri, $fb_uri);
    }
    return true;
  }
  return false;
}

/**
 * Removes a feed's output from the Drupal cache. This is used just to be sure
 * that no one is accidentally served the cached version of a feed instead of
 * being redirected.
 * @param local_uri The local system URI of the feed.
 */
/*function _feedburner_clear_feed_cache($local_uri) {
  // TODO: replace base_url with url function calls
  // Need to clear the cache for the feed page and also an alias if it is set
  // ?? cache_clear_all(url($local_uri), 'cache_page');
  global $base_url;
  cache_clear_all($base_url .'/'. $local_uri, 'cache_page');

  $alias = drupal_get_path_alias($local_uri);
  if ($alias != $local_uri) {
    cache_clear_all($base_url .'/'. $alias, 'cache_page');
  }
}*/

/**
 * Verifies a FeedBurner feed and its details and features available.
 * @param fb_uri The FeedBurner feed uri to verify.
 */
function _feedburner_verify_feed($fb_uri, $local_uri = null) {
  if ($local_uri === true) {
    $local_uri = db_result(db_query("SELECT local_uri FROM {feedburner_feeds} WHERE fb_uri = '%s'", $fb_uri));
  }

  $fb_request = drupal_http_request(_feedburner_url($fb_uri));
  if ($fb_request->code == 200) {
    $feed = array();

    // Feed ID (email subscription link) detection
    if (preg_match('/(\\d+)<\/feedburner:emailServiceId>/i', $fb_request->data, $regs)) {
      $feed['id'] = intval($regs[1]);
    }

    // Awareness API detection
    if (preg_match('/<feedburner:awareness>\\S+<\/feedburner:awareness>/i', $fb_request->data)) {
      $feed['awareness_api'] = true;
    }

    // FeedFlare detection
    if (preg_match('/feedflare/i', $fb_request->data) && preg_match('/~f\/'. $fb_uri .'/i', $fb_request->data)) {
      $feed['feedflare'] = true;
    }

    // Ad Network detection
    if (preg_match('/~a\/'. $fb_uri .'/i', $fb_request->data)) {
      $feed['ad_network'] = true;
    }

    // Update database with details
    db_query("UPDATE {feedburner_feeds} SET fid = %d, verified = %d, details = '%s' WHERE fb_uri = '%s' AND local_uri = '%s'", (isset($feed['id']) ? $feed['id'] : 0), time(), serialize($feed), $fb_uri, $local_uri);
    drupal_set_message(t('The FeedBurner feed %path was successfully verified.', array('%path' => $fb_url)));
    //return true;
  }
  else {
    db_query("UPDATE {feedburner_feeds} SET fid = 0, verified = 0, details = '' WHERE fb_uri = '%s' AND local_uri = '%s'", $fb_uri, $local_uri);
    drupal_set_message(t('The FeedBurner feed %path could not be verified. Please check that you have set up the feed in <a href="@fb-account">your FeedBurner account</a> or you can <a href="@fb-create">burn the feed at Feedburner</a>.', array('%path' => $fb_uri, '@fb-account' => 'http://www.feedburner.com/fb/a/myfeeds', '@fb-create' => 'https://www.feedburner.com/fb/a/addfeed?sourceUrl='. url($local_uri, null, null, true) .'&mappedUri='. $fb_uri)), 'error');
    //return false;
  }

  if (isset($local_uri)) {
    $local_request = drupal_http_request(url($local_uri, 'feedburner=no_redirect', null, true));
    if ($local_request->code != 200
        || !isset($local_request->headers['Content-Type'])
        || !preg_match('/xml|rss|atom|rdf/i', $local_request->headers['Content-Type'])) {
      drupal_set_message(t('The local feed source %path could not be verified. Please make sure it exists.', array('%path' => $local_uri)), 'error');
    }

    $redirect_request = drupal_http_request(url($local_uri, null, null, true));
    if (!isset($redirect_request->redirect_url)
        || $redirect_request->redirect_url != _feedburner_url($fb_uri)
        || !isset($redirect->code)
        || $redirect->code != variable_get('feedburner_redirect_status', FEEDBURNER_DEFAULT_REDIRECT_STATUS)) {
      drupal_set_message(t('Redirection from the local feed %local to the FeedBurner feed %fb could not be confirmed. Please check to make sure it works.', array('%local' => local_uri, '%fb' => $fb_uri)));
    }
  }
}

/**
 * Pings the FeedBurner service. Called when a redirection has changed.
 * @return If the ping request was successful.
 */
function _feedburner_ping() {
  $request = drupal_http_request('http://www.feedburner.com/fb/a/ping?bloglink='. urlencode(url(null, null, null, true)));
  return ($request->code == 200);
}


/**
 * Retrieve a pipe delimited string of autocomplete suggestions for feed uris.
 */
function _feedburner_autocomplete() {
  $matches = array();
  $string = _feedburner_path_keys(3);

  if ($string) {
    $result = _feedburner_list_feeds();
    $string = preg_replace(array('/\\//', '/\\./'), array('\\/', '\.'), $string);
    foreach ($result as $feed) {
      if (preg_match('/'. $string .'/i', $feed)) {
        $matches[$feed] = check_plain($feed);

      }
    }
    $matches = array_slice($matches, 0, 10);
  }

  echo drupal_to_js($matches);
  exit();
}


function _feedburner_list_feeds() {
  $feeds = array();
  $feeds[] = 'rss.xml';

  // Comment RSS integration
  if (module_exists('commentrss')) {
    $feeds[] = 'crss';
  }

  // View RSS integration
  if (module_exists('views') && module_exists('views_rss')) {
    $result = db_query("SELECT url FROM {view_view} WHERE page_type = 'views_rss'");
    while ($view_feed = db_fetch_object($result)) {
      $feeds[] = $view_feed->url;
    }
  }

  // Blog integration
  if (module_exists('blog')) {
    $feeds[] = 'blog/feed';
    // Get each user's blog feed as well
    $result = db_query('SELECT uid FROM {users} WHERE status = 1');
    while ($user_details = db_fetch_object($result)) {
      $feeds[] = 'blog/'. $user_details->uid .'/feed';
    }
  }

  // Taxonomy integration
  if (module_exists('taxonomy')) {
    $result = db_query('SELECT tid FROM {term_data}');
    while ($term_details = db_fetch_object($result)) {
      $feeds[] = 'taxonomy/term/'. $term_details->tid .'/0/feed';
    }
  }


  /*$aliases = array();
  // Change each url into it's path alias?
  foreach ($feeds as $feed) {
    $alias = drupal_get_path_alias($feed);
    if ($feed != $alias) {
      $aliases[] = $alias;
    }
  }
  $feeds = array_merge($feeds, $aliases);*/

  foreach ($feeds as &$feed) {
    $feed = drupal_get_path_alias($feed);
  }

  sort($feeds);
  return $feeds;
}