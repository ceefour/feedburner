<?php
// $Id$

function feedburner_settings_form() {
  $form['redirection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feed Redirection'),
  );
  $form['redirection']['feedburner_redirect_status'] = array(
    '#type' => 'select',
    '#title' => t('Redirection HTTP header status code'),
    '#default_value' => variable_get('feedburner_redirect_status', FEEDBURNER_DEFAULT_REDIRECT_STATUS),
    '#options' => array(
      301 => '301 Moved Permanently',
      302 => '302 Found',
      307 => '307 Temporary Redirect'),
    '#description' => t('Please note that the "307 Temporary Redirect" is recommended. For detailed descriptions of HTTP redirect status codes, see <a href="@link">the Wikipedia article on HTTP status codes</a>.', array('@link' => 'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection')),
    //'#element_validate' => array('feedburner_settings_validate'),
  );
  global $user;
  if (module_exists('blog')) {
    $form['redirection']['feedburner_blogs'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to specify a FeedBurner feed for their own blog\'s feed.'),
      '#default_value' => variable_get('feedburner_blogs', FALSE),
      '#description' => t('If the user has the permission to blog, this option is found in his/her <a href="@link">edit account</a> page. If FeedFlare is enabled for blog content types, this option will also override FeedFlare.', array('@link' => url('user/' . $user->uid . '/edit'))),
    );
  }

  $feeds = array(0 => 'None (disabled)');
  $result = db_query('SELECT DISTINCT feedburner FROM {feedburner} ORDER BY feedburner');
  while ($feed = db_result($result)) {
    $feeds[$feed] = $feed;
  }

  $form['feedflare'] = array(
    '#type' => 'fieldset',
    '#title' => t('FeedFlare/Standard Stats/Ad Network'),
    '#description' => t('See <a href="@link">@link</a> for more information.', array('@link' => 'http://www.feedburner.com/fb/a/publishers/feedflare')),
    '#collapsible' => FALSE,
  );
  $form['feedflare']['feedburner_feedflare_feed'] = array(
    '#type' => 'select',
    '#title' => t('FeedFlare/Standard Stats feed'),
    '#default_value' => variable_get('feedburner_feedflare_feed', 0),
    '#options' => $feeds,
    //'#element_validate' => array('feedburner_settings_validate'),
  );
  $form['feedflare']['feedburner_feedflare_node_types'] = array(
    '#type' => 'checkboxes',
    //'#type' => 'select',
    '#title' => t('Node types'),
    '#multiple' => TRUE,
    '#default_value' => variable_get('feedburner_feedflare_node_types', array()),
    '#options' => node_get_types('names'),
  );
  $form['feedflare']['feedburner_feedflare_display'] = array(
    '#type' => 'select',
    '#title' => t('FeedFlare/Ad Network in nodes'),
    '#default_value' => variable_get('feedburner_feedflare_display', 'page'),
    '#options' => array(
      // TODO A better way to list node display types?
      'teaser' => t('Teaser view'),
      'page' => t('Full-page view'),
      'both' => t('Teasers and full-page view')
    ),
    '#description' => t('When to display the FeedFlare after the node text. There is also a <a href="@block">FeedFlare block available</a>.', array('@block' => url('admin/build/block/configure/feedburner/0'))),
  );
  //$form['feedflare']['feedburner_standardstats'] = array(
  //  '#type' => 'checkbox',
  //  '#default_value' => variable_get('feedburner_standardstats', TRUE),
  //  '#title' => t('Include StandardStats code on pages that don\'t display FeedFlare or Ad Network'),
  //);

  $form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('FeedBurner Account'),
    '#description' => t('To use more advanced features like FeedFlare, e-mail subscriptions, and feed stats, you need to enter your FeedBurner account details. If you do not yet have a FeedBurner account, you can register at <a href="@link">@link</a>.', array('@link' => 'https://www.feedburner.com/fb/a/register')),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $auth = variable_get('feedburner_auth', NULL);
  $form['account']['feedburner_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Your FeedBurner username'),
    '#default_value' => (!empty($auth) ? strtok(base64_decode($auth), ':') : ''),
    '#size' => 25,
    '#maxlength' => 30,
    //'#element_validate' => array('feedburner_settings_validate'),
  );
  $form['account']['feedburner_password'] = array(
    '#type' => 'password',
    '#title' => t('Your FeedBurner password'),
    //'#default_value' => '',
    '#size' => 25,
    '#maxlength' => 30,
    //'#element_validate' => array('feedburner_settings_validate'),
  );
  $form['account']['verified'] = array(
    '#type' => 'markup',
    '#value' => '<p><strong>' . t('Account status:') . '</strong><br /><span style="color: ' . ($auth ? 'green">' . t('Verified') : 'red">' . t('NOT VERIFIED')) . '</span></p>',
  );

  if (!_feedburner_can_api()) {
    $form['account']['#title'] .= t(' (DISABLED - requires PHP 5)');
    $form['account']['#collapsed'] = TRUE;
    $form['account']['feedburner_username']['#disabled'] = TRUE;
    $form['account']['feedburner_password']['#disabled'] = TRUE;
  }

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['advanced']['feedburner_useragents'] = array(
    '#type' => 'textarea',
    '#title' => t('User agents allowed direct feed access'),
    '#description' => t('Enter one page per line as user agents that will be allowed to access your site\'s feeds without being redirected to FeedBurner. The \'*\' character is a wildcard. User agent strings do not need to be exact (e.g. "feed" will match "feedburner").'),
    '#default_value' => preg_replace(array('/(?<!\\\\)\|/', '/\\.\\*/', '/(?<!\\\)\\\/'), array("\n", '*', ''), variable_get('feedburner_useragents', FEEDBURNER_DEFAULT_USERAGENTS)),
    '#rows' => 3,
    //'#element_validate' => array('feedburner_settings_validate'),
  );
  $form['advanced']['feedburner_path_blacklist'] = array(
    '#type' => 'textarea',
    '#title' => t('Redirection path blacklist'),
    '#description' => t('If the current path matches one of these paths, this module will not check to see if the current path is a feed and should be redirected. Helps save a little processing time on every hit, including cached view. Enter one page per line as Drupal paths. The "*" character is a wildcard and @front is the frontpage. Paths need to be exact (e.g. "node" will not match "node/1", but "node/*" will). To disable redirection temporarly, use just "*" as a path.', array('@front' => '<front>')),
    '#default_value' => preg_replace(array('/(?<!\\\\)\|/', '/\\.\\*/', '/(?<!\\\)\\\/', '/' . variable_get('site_frontpage', 'node') . '/'), array("\n", '*', '', '<front>'), variable_get('feedburner_path_blacklist', FEEDBURNER_DEFAULT_PATH_BLACKLIST)),
    '#rows' => 4,
    //'#element_validate' => array('feedburner_settings_validate'),
  );
  $form['advanced']['feedburner_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('MyBrand custom domain'),
    '#description' => t('MyBrand is a premium FeedBurner service that allows you to showcase your feeds by serving them from your own domain (e.g., @domain). You must have access to change your CNAME entry in the DNS records for the domain you wish to change. For more information, see <a href="@link">@link</a>.', array('@link' => 'http://www.feedburner.com/fb/a/mybrand', '@domain' => 'feeds.' . $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('feedburner_domain', FEEDBURNER_DEFAULT_DOMAIN),
    '#field_prefix' => 'http://',
    '#field_suffix' => '/feedname',
    '#size' => 30,
    '#maxlength' => 30,
    //'#element_validate' => array('feedburner_settings_validate'),
  );

  //$form['array_filter'] = array('#type' => 'hidden');
  $form['array_filter'] = array('#type' => 'value', '#value' => TRUE);
  $form['blurb'] = array(
    '#type' => 'markup',
    '#value' => _feedburner_blurb(),
    '#weight' => 10,
  );

  return system_settings_form($form);
}

//function feedburner_settings_validate($form_element, &$form_state) {
//  $element_value = $form_element['#value'];
//  $element_name = $form_element['#name'];
//  switch ($element_name) {
//    case 'feedburner_redirect_status':
//
//      break;
//  }
//  if ($element_value != $form_element['#value']) {
//    $form_state['values'][$element_name] = $element_value;
//  }
//}

function feedburner_settings_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP);

  // feedburner username

  // feedburner password

  // feedburner account validate

  // feedburner_useragents
  //echo "<pre>";
  //echo($feedburner_useragents); echo "\n\n";
  $feedburner_useragents = preg_quote(trim($feedburner_useragents), '/');
  if (empty($feedburner_useragents)) {
    form_set_error('feedburner_useragents', 'Please enter at least one user agent that will have access to your site\'s feeds. It is strongly recommended to allow both the "feedburner" and "feedvalidator" user agents.');
    return;
  }
  if (strpos($feedburner_useragents, 'feedburner') === FALSE || strpos($feedburner_useragents, 'feedvalidator') === FALSE) {
    drupal_set_message(t('It is SERIOUSLY recommended to allow both the "feedburner" and "feedvalidator" user agents access to your site\'s feed. Failure to do so may result in catastrophic failure, or things just not working.'), 'error');
  }
  $feedburner_useragents = preg_replace(array('/(\r\n?|\n)+/', '/\\\\\*/', '/^\\|/', '/\\|$/'), array('|', '.*', '', ''), $feedburner_useragents);
  //echo($feedburner_useragents); echo "\n\n";
  //$feedburner_useragents = preg_replace(array('/(?<!\\\\)\|/', '/\\.\\*/', '/(?<!\\\)\\\/'), array("\n", '*', ''), $feedburner_useragents);
  //echo($feedburner_useragents); die();

  // feedburner_path_blacklist
  //echo "<pre>";
  //echo($feedburner_path_blacklist); echo "\n1\n";
  $feedburner_path_blacklist = preg_quote(trim($feedburner_path_blacklist), '/');
  $feedburner_path_blacklist = preg_replace(array('/(\r\n?|\n)+/', '/\\\\\*/', '/^\\|/', '/\\|$/', '/(^|\|)\\\\<front\\\\>($|\|)/'), array('|', '.*', '', '', '\1' . preg_quote(variable_get('site_frontpage', 'node'), '/') . '\2'), $feedburner_path_blacklist);
  //echo($feedburner_path_blacklist); echo "\n2\n";
  //$feedburner_path_blacklist = preg_replace(array('/(?<!\\\\)\|/', '/\\.\\*/', '/(?<!\\\)\\\/', '/' . variable_get('site_frontpage', 'node') . '/'), array("\n", '*', '', '<front>'), $feedburner_path_blacklist);
  //echo($feedburner_path_blacklist); die();

  // feedburner_domain
  if ($feedburner_domain != variable_get('feedburner_domain', FEEDBURNER_DEFAULT_DOMAIN) && $feedburner_domain != FEEDBURNER_DEFAULT_DOMAIN) {
    $request = drupal_http_request('http://' . $feedburner_domain . '/');
    unset($request->data);
    if (!isset($request->headers['X-FB-Host']) || !isset($request->code) || $request->code != 400 || !isset($request->error) || $request->error != 'You must provide a feed uri') {
      //TODO Insert help link for CNAME MyBrand editing (http://www.feedburner.com/fb/a/publishers/mybrand)
      drupal_set_message(t('The MyBrand domain <a href="http://@domain/">@domain</a> failed verification. Make sure you have edited the domain\'s CNAME settings before enabling this option.', array('@domain' => $feedburner_domain)), 'error');
    }
  }

  $form_state['values'] = array_merge($form_state['values'], compact(array('feedburner_useragents', 'feedburner_path_blacklist', 'feedburner_domain')));

  unset($form_state['values']['feedburner_password']);
}


function _feedburner_save_feed($path, $feedburner = NULL, $verify = FALSE) {
  if (empty($feedburner)) {
    db_query("DELETE FROM {feedburner} WHERE src = '%s'", $path);
  }
  else {
    db_query("UPDATE {feedburner} SET feedburner = '%s' WHERE path = '%s'", $feedburner, $path);
    if (db_affected_rows() == 0) {
      db_query("INSERT INTO {feedburner} (path, feedburner) VALUES ('%s', '%s')", $path, $feedburner);
    }
    if ($verify) {
      //_feedburner_verify_feed($path, $feedburner);
    }
  }
  return (db_affected_rows() != 0);
}


function _feedburner_get_feeds($category, $options = array()) {
  if ($category == 'types') {
    return array('site', 'blogs', 'taxonomy');
  }

  $feeds = array();
  $default_options = array('sort' => TRUE, 'normalize' => TRUE, 'feedburner' => TRUE, 'fields' => array());
  $options = array_merge($default_options, $options);

  if (preg_match('/all|site/', $category)) {
    // Default Site RSS
    $feeds['rss.xml'] = array('path' => 'rss.xml', 'description' => variable_get('site_name', 'Drupal') . ' RSS');
    $feeds['test'] = array('path' => 'test');

    // Comment RSS
    if (module_exists('commentrss')) {
      $feeds['crss'] = array('path' => 'crss', 'description' => variable_get('site_name', 'Drupal') . ' Comments');
    }

    // Atom Feeds
    if (module_exists('atom')) {
      $feeds['atom/feed'] = array('path' => 'atom/feed', 'description' => variable_get('site_name', 'Drupal') . ' Atom');
    }

    // Event integration
    if (module_exists('event')) {
      $feeds['event/feed'] = array('path' => 'event/feed');
    }
  }

  // Blog integration
  if (preg_match('/all|blogs/', $category) && module_exists('blog')) {
    $feeds['blog/feed'] = array('path' => 'blog/feed', 'category' => 'blogs', 'description' => 'Blogs');
    // Get all user blog feeds for users with the blogging permission granted
    $blog_roles = user_roles(TRUE, 'create blog entries');
    $sql = "SELECT u.uid, u.name FROM {users} u LEFT JOIN {users_roles} ur ON ur.uid = u.uid LEFT JOIN {role} r ON r.rid = ur.rid WHERE u.uid = '1'" . (empty($blog_roles) ? '' : " OR (u.status =  '1' AND r.name IN ( " . db_placeholders($blog_roles, 'text') . '))');
    $result = db_query($sql, $blog_roles);
    while ($user_details = db_fetch_object($result)) {
      $path = 'blog/' . $user_details->uid . '/' . variable_get('pathauto_blog_supportsfeeds', 'feed');
      $feeds[$path] = array('path' => $path, 'category' => 'blogs', 'description' => $user_details->name . '\'s Blog');
    }
    unset($result);
  }

  // Taxonomy integration
  if (preg_match('/all|taxonomy/', $category) && module_exists('taxonomy')) {
    $result = db_query('SELECT tid, name FROM {term_data}');
    while ($term_details = db_fetch_object($result)) {
      $path = 'taxonomy/term/' . $term_details->tid . '/' . variable_get('pathauto_taxonomy_supportsfeeds', 'feed');
      $feeds[$path] = array('path' => $path, 'category' => 'taxonomy', 'description' => $term_details->name);
    }
    unset($result);
  }

  // TODO: Views integration
  //if (preg_match('/all|views/', $category) && module_exists('views')) {
  //  $result = db_query("SELECT url, page_title FROM {view_view} WHERE page_type = 'views_rss' OR page_type = 'views_podcast'");
  //  while ($view_details = db_fetch_object($result)) {
  //    $feeds[$view_details->url] = array('path' => $view_details->url, 'category' => 'views', 'description' => $view_details->page_title);
  //  }
  //  $result = db_query("SELECT v.url, v.page_title FROM {view_view} v, {view_argument} a WHERE a.type = 'rss_feed' and v.vid = a.vid");
  //  while ($viewargs_details = db_fetch_object($result)) {
  //    $path = $viewargs_details->url . '/feed';
  //    $feeds[$path] = array('path' => $path, 'category' => 'views', 'description' => $viewargs_details->page_title);
  //  }
  //  unset($result);
  //}

  // TODO: Tracker integration
  //$result = db_query('SELECT uid, name FROM {users} WHERE status = 1');
  //while ($user_details = db_fetch_object($result)) {
  //  $path = 'user/' . $user_details->uid . '/track/' . variable_get('pathauto_blog_supportsfeeds', 'feed');
  //  $feeds[$path] = array('uri' => $path, 'category' => 'users', 'description' => $user_details->name . '\'s Tracker');
  //}
  //unset($result);

  // TODO: Content integration
  //$result = db_query('SELECT nid, title FROM {node} WHERE status = 1');
  //while ($node_details = db_fetch_object($result)) {
  //  $path = 'node/' . $node_details->nid . '/' . variable_get('pathauto_node_supportsfeeds', 'feed');
  //  $feeds[$node_feed] = array('uri' => $path, 'category' => 'content', 'description' => $node_details->title);
  //}
  //unset($result);

  // TODO Use module_invoke to get feeds from other modules?

  // Merge FeedBurner data
  if ($options['feedburner'] || $category == 'feedburner') {
    $result = db_query('SELECT path, feedburner, verified FROM {feedburner}');
    while ($feed = db_fetch_array($result)) {
      $path = $feed['path'];
      if (preg_match('/all|site|feedburner/', $category) || isset($feeds[$path]) == $options['feedburner']) {
        if (!isset($feeds[$path])) {
          $feeds[$path] = $feed;
        }
        else {
          $feeds[$path] = array_merge($feeds[$path], $feed);
        }
      }
    }
  }

  // Assign generic 'site' category to un-categorized feeds and get feed aliases if requested
  foreach ($feeds as &$feed) {
    if (!isset($feed['category'])) {
      $feed['category'] = 'site';
    }
  }
  unset($feed);

  // Normalize paths (and array keys)
  if ($options['normalize']) {
    $keys = array();
    foreach ($feeds as &$feed) {
      $alias = drupal_get_path_alias($feed['path']);
      $keys[] = $alias;
      if ($feed['path'] != $alias) {
        $feed['alias'] = $alias;
      }
    }
    unset($feed);

    //$keys = array_keys($feeds);
    //$keys = array_map('drupal_get_path_alias', $keys);
    $values = array_values($feeds);
    $feeds = array_combine($keys, $values);
  }

  // Sort by array key (custom sort)
  if ($options['sort']) {
    function _feedburner_temp_sort($key1, $key2) {
      $key1 = explode('/', $key1, 2);
      $key2 = explode('/', $key2, 2);
      if (intval($key1[0]) xor intval($key2[0])) {
        return intval($key1[0]);
      }
      elseif ($result = strnatcasecmp($key1[0], $key2[0])) {
        return $result;
      }
      else {
        return _feedburner_temp_sort($key1[1], $key2[1]);
      }
    }
    uksort($feeds, '_feedburner_temp_sort');
  }

  return $feeds;
}


function _feedburner_verify_feed($local, $feedburner) {
  for ($i = 0; $i < 3; $i++) {
    $url = NULL;
    $request = NULL;
    $message_start = NULL;
    $result = array(NULL, NULL, NULL, time());
    switch ($i) {
      case 0:
        $url = url($local, array('absolute' => TRUE, 'query' => 'redirect=no'));
        $message_start = t('Local feed ');
        break;
      case 1:
        $url = _feedburner_construct_url($feedburner);
        $message_start = t('FeedBurner feed ');
        break;
      case 2:
        $url = url($local, array('absolute' => TRUE));
        $message_start = t('Local feed redirection for ');
        break;
    }

    $request = drupal_http_request($url);
    unset($request->data);

    switch ($i) {
      case 0:
        $url = url($local, array('absolute' => TRUE));
      case 1:
        $result[$i] = (isset($request) && isset($request->code) && $request->code == '200'
        && is_array($request->headers) && preg_match('/rss|atom|xml/', $request->headers['Content-Type']));
        break;
      case 2:
        $result[$i] = (isset($request) && isset($request->code)
          && $request->code == variable_get('feedburner_redirect_status', FEEDBURNER_DEFAULT_REDIRECT_STATUS)
          && isset($request->redirect_code) && $request->redirect_code == '200'
          && isset($request->redirect_url) && $request->redirect_url == _feedburner_construct_url($feedburner));
        break;
    }

    if ($result[$i]) {
      drupal_set_message($message_start . t('<a href="@link">@link</a> verified.', array('@link' => $url)));
    }
    else {
      $result[3] = 0;
      drupal_set_message($message_start . t('<a href="@link">@link</a> failed verification. @error', array('@link' => $url, '@error' => $request->error)), 'error');
    }
  }

  db_query("UPDATE {feedburner} SET verified = %d WHERE path = '%s' AND feedburner = '%s'", $result[3], $local, $feedburner);
}