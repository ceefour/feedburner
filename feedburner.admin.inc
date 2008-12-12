<?php
// $Id$

/**
 * @file
 * Administrative functions and page callbacks for the feedburner module.
 */

function feedburner_build_overview($cat) {
  $keys = _feedburner_get_path(6);
  $output = drupal_get_form('feedburner_build_filter_form', $cat, $keys);

  $header = array(
    array('data' => t('Local path')),
    array('data' => t('FeedBurner')),
    array('data' => t('Operations'), 'colspan' => '3'),
  );

  $feeds = _feedburner_get_feeds($cat, array('match' => $keys));
  $row_limit = 20;
  $feeds = _feedburner_build_pager($feeds, $row_limit);

  $rows = array();
  foreach ($feeds as $feed) {
    $row = array();
    //$row[] = l($feed['path'] . (isset($feed['alias']) ? ' (' . $feed['alias'] . ')' : ''), $feed['path']);
    $row[] = l(($feed['alias'] != $feed['path'] ? $feed['alias'] . ' (' . $feed['path'] . ')' : $feed['path']), $feed['path']);
    if (isset($feed['feedburner'])) {
      $row[] = l($feed['feedburner'], _feedburner_construct_url($feed['feedburner']));
      $row[] = l(t('Edit'), 'admin/build/feedburner/burn/' . $feed['path']);
      $row[] = l(t('Unburn'), 'admin/build/feedburner/unburn/' . $feed['path']);
      $row[] = l(t('Source'), $feed['path'], array('query' => 'redirect=no'));
    }
    else {
      $row[] = '';
      $row[] = l(t('Burn'), 'admin/build/feedburner/burn/' . $feed['path']);
      $row[] = '';
      $row[] = '';
    }
    $rows[] = $row;
  }

  if (empty($rows)) {
    $empty_message = $keys ? t('No feeds found.')  : t('No feeds available.') ;
    $rows[] = array(array('data' => $empty_message, 'colspan' => 5));
  }

  $output .= theme('table', $header, $rows);
  $output .= theme('pager', NULL, $row_limit);
  $output .= _feedburner_blurb();
  return $output;
}

/**
 * Copy of the pager_query function, meant to work on arrays.
 */
function _feedburner_build_pager($array, $limit = 10) {
  global $pager_page_array, $pager_total, $pager_total_items;
  $page = isset($_GET['page']) ? $_GET['page'] : '';
  $pager_page_array = explode(',', $page);
  $pager_total_items[0] = count($array);
  $pager_total[0] = ceil($pager_total_items[0] / $limit);
  $pager_page_array[0] = max(0,  min((int)$pager_page_array[0], ((int)$pager_total[0]) - 1));
  return array_splice($array, $pager_page_array[0] * $limit, $limit);
}

/**
 * Return a form to filter URL aliases.
 */
function feedburner_build_filter_form(&$form_state, $cat, $keys = '') {
  $form['#attributes'] = array('class' => 'search-form');
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
    '#submit' => array('feedburner_build_filter_form_submit_filter'),
  );
  $form['cat'] = array(
    '#type' => 'value',
    '#value' => $cat,
  );
  if (!empty($keys)) {
    $form['basic']['inline']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#submit' => array('feedburner_build_filter_form_submit_reset'),
    );
  }
  return $form;
}

/**
 * Process filter form submission when the Filter button is pressed.
 */
function feedburner_build_filter_form_submit_filter($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/feedburner/list/' . $form_state['values']['cat'] . '/' . trim($form_state['values']['filter']);
}

/**
 * Process filter form submission when the Reset button is pressed.
 */
function feedburner_build_filter_form_submit_reset($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/feedburner/list/' . $form_state['values']['cat'];
}


function feedburner_burn_form() {
  $path = _feedburner_get_path(5);
  $feedburner = db_result(db_query("SELECT feedburner FROM {feedburner} WHERE path = '%s'", $path));

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Local Feed'),
    '#default_value' => $path,
    '#maxlength' => 128,
    '#size' => 45,
    '#description' => t('Specify the existing feed on your site from which to redirect.'),
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
    '#autocomplete_path' => 'admin/feedburner/autocomplete/all',
  );
  $form['feedburner'] = array(
    '#type' => 'textfield',
    '#title' => t('FeedBurner Feed'),
    '#default_value' => $feedburner,
    '#maxlength' => 100,
    '#size' => 45,
    '#description' => t('Requests to the above feed will be requested to this FeedBurner feed. This field is case-sensitive and alphanumeric.'),
    '#field_prefix' => _feedburner_construct_url(),
    '#autocomplete_path' => 'admin/feedburner/autocomplete/feedburner',
  );

  //$form['options'] = array(
  //  '#type' => 'fieldset',
  //  '#title' => t('Options'),
  //  '#collapsible' => TRUE,
  //  '#description' => t('FeedBurner Management API features coming soon.'),
  //);
  // TODO: Feed Management API Options

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['cancel'] = array('#value' => l(t('Cancel'), 'admin/build/feedburner'));

  return $form;
}

function feedburner_burn_form_validate($form, &$form_state) {
  $path = $form_state['values']['path'];
  $path = drupal_get_normal_path($path);
  $path_menu = menu_get_item($path);
  if (!valid_url($path) || !$path_menu || !$path_menu['access']) {
    form_set_error('path', t("The path '@link_path' is either invalid or you do not have access to it.", array('@link_path' => $path)));
  }

  $feedburner = $form_state['values']['feedburner'];
  if (preg_match('%[^\w-/]%', $feedburner)) {
    form_set_error('feedburner', 'Invalid FeedBurner feed name');
  }
}

function feedburner_burn_form_submit($form, &$form_state) {
  _feedburner_save_feed($form_state['values']['path'], $form_state['values']['feedburner']);
  drupal_set_message(t('The feed redirection for %path has been saved.', array('%path' => $form_state['values']['path'])));
  if (_feedburner_can_api(TRUE)) {
    // TODO: AddFeed / ModifyFeed Management API Calls
  }
  $form_state['redirect'] = 'admin/build/feedburner';
  return;
}


function feedburner_unburn_form() {
  $path = _feedburner_get_path(5);
  $feed = db_result(db_query("SELECT feedburner FROM {feedburner} WHERE path = '%s'", $path));

  if (empty($path) || empty($feed)) {
    drupal_goto('admin/build/feedburner');
    exit();
  }

  $form['path'] = array(
    '#type' => 'hidden',
    '#value' => $path,
  );
  //$form['unburn'] = array(
  //  '#type' => 'checkbox',
  //  '#title' => t('Delete the FeedBurner feed %feed from my FeedBurner account.', array('%feed' => $feed->fb_uri)),
  //  '#default_value' => FALSE,
  //  '#disabled' => TRUE,
  //  '#description' => t('FeedBurner Management API features coming soon.'),
  //);
  //
  //if (!_feedburner_can_api(TRUE)) {
  //  $form['unburn']['#disabled'] = TRUE;
  //}

  $form['#redirect'] = 'admin/build/feedburner';

  return confirm_form($form, t('Are you sure you want to unburn %feed?', array('%feed' => $path)), 'admin/build/feedburner', t('Requests for this feed will no longer be redirected to FeedBurner. But don\'t worry, you can always re-burn this feed to FeedBurner.'));
}

function feedburner_unburn_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    if ($form_state['values']['unburn']) {
      // TODO DeleteFeed API implementation for unburn feed submit
    }
    _feedburner_save_feed($form_state['values']['path']);
    drupal_set_message(t('Redirection for %url has been removed.', array('%url' => $form_state['values']['path'])));
    $form_state['redirect'] = 'admin/build/feedburner';
    return;
  }
}

function feedburner_settings_form() {
  $form['redirection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feed Redirection'),
  );
  $form['redirection']['feedburner_redirect_status'] = array(
    '#type' => 'select',
    '#title' => t('Redirection HTTP header status code'),
    '#default_value' => variable_get('feedburner_redirect_status', 307),
    '#options' => array(
      301 => '301 Moved Permanently',
      302 => '302 Found',
      307 => '307 Temporary Redirect'),
    '#description' => t('Please note that the "307 Temporary Redirect" is recommended. For detailed descriptions of HTTP redirect status codes, see <a href="@link">the Wikipedia article on HTTP status codes</a>.', array('@link' => 'http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#3xx_Redirection')),
  );
  if (module_exists('blog')) {
    $form['redirection']['feedburner_blogs'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to specify a FeedBurner feed for their own blog\'s feed.'),
      '#default_value' => variable_get('feedburner_blogs', 0),
      '#description' => t('If the user has the permission to blog, this option is found in his/her <a href="@link">edit account</a> page. If FeedFlare is enabled for blog content types, this option will also override FeedFlare.', array('@link' => url('user/' . $GLOBALS['user']->uid . '/edit'))),
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
    '#title' => t('FeedFlare/Standard Stats/Ad Network feed'),
    '#default_value' => variable_get('feedburner_feedflare_feed', 0),
    '#options' => $feeds,
  );
  $form['feedflare']['feedburner_feedflare_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Display in node types'),
    '#default_value' => variable_get('feedburner_feedflare_node_types', array()),
    '#options' => node_get_types('names'),
  );
  $form['feedflare']['feedburner_feedflare_display'] = array(
    '#type' => 'checkboxes',
    '#title' => t('How to display FeedFlare/Ad Network'),
    '#default_value' => variable_get('feedburner_feedflare_display', array('page')),
    '#options' => array(
      'teaser' => t('Teaser view'),
      'page' => t('Full-page view'),
    ), // TODO: A better way to list node display types?
    '#description' => t('When to display the FeedFlare after the node text. There is also a <a href="@block">FeedFlare block available</a>.', array('@block' => url('admin/build/block/configure/feedburner/0'))),
  );
  //$form['feedflare']['feedburner_standardstats'] = array(
  //  '#type' => 'checkbox',
  //  '#default_value' => variable_get('feedburner_standardstats', TRUE),
  //  '#title' => t('Include StandardStats code on pages that don\'t display FeedFlare or Ad Network'),
  //);

  //$form['account'] = array(
  //  '#type' => 'fieldset',
  //  '#title' => t('FeedBurner Account'),
  //  '#description' => t('To use more advanced features like FeedFlare, e-mail subscriptions, and feed stats, you need to enter your FeedBurner account details. If you do not yet have a FeedBurner account, you can register at <a href="@link">@link</a>.', array('@link' => 'https://www.feedburner.com/fb/a/register')),
  //  '#collapsible' => TRUE,
  //  '#collapsed' => FALSE,
  //);
  //$auth = variable_get('feedburner_auth', NULL);
  //$form['account']['feedburner_username'] = array(
  //  '#type' => 'textfield',
  //  '#title' => t('Your FeedBurner username'),
  //  '#default_value' => (!empty($auth) ? strtok(base64_decode($auth), ':') : ''),
  //  '#size' => 25,
  //  '#maxlength' => 30,
  //);
  //$form['account']['feedburner_password'] = array(
  //  '#type' => 'password',
  //  '#title' => t('Your FeedBurner password'),
  //  //'#default_value' => '',
  //  '#size' => 25,
  //  '#maxlength' => 30,
  //);
  //$form['account']['verified'] = array(
  //  '#type' => 'markup',
  //  '#value' => '<p><strong>' . t('Account status:') . '</strong><br /><span style="color: ' . ($auth ? 'green">' . t('Verified') : 'red">' . t('NOT VERIFIED')) . '</span></p>',
  //);
  //
  //if (!_feedburner_can_api()) {
  //  $form['account']['#title'] .= t(' (DISABLED - requires PHP 5)');
  //  $form['account']['#collapsed'] = TRUE;
  //  $form['account']['feedburner_username']['#disabled'] = TRUE;
  //  $form['account']['feedburner_password']['#disabled'] = TRUE;
  //}

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
    '#default_value' => preg_replace(array('%^/(.*)/i$%', '/(?<!\\\\)\|/', '/\.\*/', '/(?<!\\\\)\\\\/'), array('\1', "\n", '*', ''), variable_get('feedburner_useragents', '/feedburner|feedvalidator/i')),
    '#rows' => 3,
  );
  $form['advanced']['feedburner_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('MyBrand custom domain'),
    '#description' => t('MyBrand is a premium FeedBurner service that allows you to showcase your feeds by serving them from your own domain (e.g., @domain). You must have access to change your CNAME entry in the DNS records for the domain you wish to change. For more information, see <a href="@link">@link</a>.', array('@link' => 'http://www.feedburner.com/fb/a/mybrand', '@domain' => 'feeds.' . $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('feedburner_domain', 'feeds.feedburner.com'),
    '#field_prefix' => 'http://',
    '#field_suffix' => '/feedname',
    '#size' => 30,
    '#maxlength' => 30,
  );

  $form['array_filter'] = array('#type' => 'value', '#value' => TRUE);
  $form['blurb'] = array(
    '#type' => 'markup',
    '#value' => _feedburner_blurb(),
    '#weight' => 10,
  );

  return system_settings_form($form);
}


function feedburner_settings_form_validate($form, &$form_state) {
  $values =& $form_state['values'];

  // Fix redirect code to int
  $values['feedburner_redirect_status'] = (int) $values['feedburner_redirect_status'];

  //// feedburner username && password
  //if (!preg_match('/^\w*$/', $values['feedburner_username'])) {
  //  form_set_error('feedburner_username', 'Invalid FeedBurner username.');
  //}
  //if (!preg_match('/^\w*$/', $values['feedburner_password'])) {
  //  //TODO: Can match whitespace characters
  //  form_set_error('feedburner_username', 'Invalid FeedBurner password.');
  //}
  // unset username and password

  // feedburner_useragents
  $useragents = preg_replace('/^[|\s]+|[|\s]+$/m', '', $values['feedburner_useragents']);
  $useragents = preg_quote($useragents);
  if (empty($useragents) || !preg_match('/\w+/', $useragents)) {
    drupal_set_message(t('Please enter at least one user agent that will have access to your site\'s feeds. It is strongly recommended to allow both the "feedburner" and "feedvalidator" user agents.'), 'error');
  }
  $useragents = '/' . preg_replace(array('/[\r\n]+/', '/\\\\\*/'), array('|', '.*'), $useragents) . '/i';
  $values['feedburner_useragents'] = $useragents;

  // feedburner_domain
  $feedburner_domain = $values['feedburner_domain'];
  if (!variable_get('drupal_http_request_fails', FALSE) && $feedburner_domain != variable_get('feedburner_domain', 'feeds.feedburner.com') && $feedburner_domain != 'feeds.feedburner.com') {
    $request = drupal_http_request('http://' . $feedburner_domain . '/');
    unset($request->data);
    if (!isset($request->headers['X-FB-Host']) || !isset($request->code) || $request->code != 400 || !isset($request->error) || $request->error != 'You must provide a feed uri') {
      // TODO: Insert help link for CNAME MyBrand editing (http://www.feedburner.com/fb/a/publishers/mybrand)
      drupal_set_message(t('The MyBrand domain <a href="http://@domain/">@domain</a> failed verification. Make sure you have edited the domain\'s CNAME settings before enabling this option.', array('@domain' => $feedburner_domain)), 'error');
    }
  }
}


function _feedburner_save_feed($path, $feedburner = NULL) {
  $current = db_result(db_query("SELECT feedburner FROM {feedburner} WHERE path = '%s'", $path));
  if (empty($feedburner) && !empty($current)) {
    db_query("DELETE FROM {feedburner} WHERE path = '%s'", $path);
    if ($current == variable_get('feedburner_feedflare_feed', 0)) {
      variable_set('feedburner_feedflare_feed', 0);
    }
  }
  else {
    if (!empty($current) && $current != $feedburner) {
      db_query("UPDATE {feedburner} SET feedburner = '%s' WHERE path = '%s'", $feedburner, $path);
    }
    elseif (empty($current)) {
      db_query("INSERT INTO {feedburner} (path, feedburner) VALUES ('%s', '%s')", $path, $feedburner);
    }
    // TODO: Check verification of feed?
  }
  return (db_affected_rows() != 0);
}


// TODO: Split into _get_feeds and _process_feeds?
function _feedburner_get_feeds($cat = 'site', $options = array()) {
  $default_options = array('sort' => TRUE, 'alias' => TRUE, 'feedburner' => TRUE, 'match' => FALSE, 'key' => FALSE, 'fields' => array());
  $options = array_merge($default_options, $options);
  $feeds = array();

  // Site feeds (and various module feeds)
  if ($cat == 'site' || $cat == 'all') {
    $feeds[] = array('path' => 'rss.xml', 'description' => variable_get('site_name', 'Drupal') . ' RSS');
    $feeds[] = array('path' => 'aggregator/feed', 'description' => 'Aggregator RSS', 'module' => 'aggregator');
    //$feeds[] = array('path' => 'blog/feed', 'description' => 'Blogs', 'module' => 'blog');
    $feeds[] = array('path' => 'crss', 'description' => variable_get('site_name', 'Drupal') . ' Comments', 'module' => 'commentrss');
    $feeds[] = array('path' => 'atom/feed', 'description' => variable_get('site_name', 'Drupal') . ' Atom', 'module' => 'atom');
  }

  // TODO: Views integration
  if (($cat == 'site' || $cat == 'all') && module_exists('views')) {
    $views = db_query("SELECT display_options FROM {views_display} WHERE display_plugin = 'feed'");
  }
  //if (preg_match('/all|views/', $cat) && module_exists('views')) {
  //  $result = db_query("SELECT url, page_title FROM {view_view} WHERE page_type = 'views_rss' OR page_type = 'views_podcast'");
  //  while ($view_details = db_fetch_object($result)) {
  //    $feeds[] = array('path' => $view_details->url, 'cat' => 'views', 'description' => $view_details->page_title);
  //  }
  //  $result = db_query("SELECT v.url, v.page_title FROM {view_view} v, {view_argument} a WHERE a.type = 'rss_feed' and v.vid = a.vid");
  //  while ($viewargs_details = db_fetch_object($result)) {
  //    $feeds[] = array('path' => $viewargs_details->url . '/feed', 'cat' => 'views', 'description' => $viewargs_details->page_title);
  //  }
  //  unset($result);
  //}

  // TODO: Content integration
  //$result = db_query('SELECT nid, title FROM {node} WHERE status = 1');
  //while ($node_details = db_fetch_object($result)) {
  //  $feeds[] = array('path' => 'node/' . $node_details->nid . '/feed', 'cat' => 'content', 'description' => $node_details->title);
  //}
  //unset($result);

  // Blog module feeds
  if (($cat == 'all' || $cat == 'blogs') && module_exists('blog')) {
    $feeds[] = array('path' => 'blog/feed', 'description' => variable_get('site_name', 'Drupal') . ' blogs');

    // Get all user blog feeds for users with the blogging permission granted
    $blog_roles = user_roles(TRUE, 'create blog entries');
    $blog_roles_sql = (count($blog_roles) ? (isset($blog_roles[2]) ? ' OR u.uid > 1' : ' OR ur.rid IN (' . db_placeholders($blog_roles) . ')') : '');
    $sql = "SELECT u.uid, u.name FROM {users} u LEFT JOIN {users_roles} ur ON ur.uid = u.uid WHERE u.uid = 1" . $blog_roles_sql;
    $result = db_query($sql, array_keys($blog_roles));
    while ($user_details = db_fetch_object($result)) {
      $feeds[] = array('path' => 'blog/' . $user_details->uid . '/feed', 'description' => $user_details->name . "'s Blog");
    }
    unset($result);
  }

  // Taxonomy feeds
  if (($cat == 'all' || $cat == 'taxonomy') && module_exists('taxonomy')) {
    $result = db_query('SELECT tid, name FROM {term_data}');
    while ($term_details = db_fetch_object($result)) {
      $feeds[] = array('path' => 'taxonomy/term/' . $term_details->tid . '/0/feed', 'description' => $term_details->name);
    }
    unset($result);
  }

  // FeedBurner feeds
  // TODO: Fetch FeedBurner feeds from FeedManagement API
  $feedburner_feeds = array();
  if ($cat == 'feedburner' || $options['feedburner']) {
    $query = db_query("SELECT path, feedburner FROM {feedburner}");
    while ($feed = db_fetch_array($query)) {
      if ($cat == 'all' || $cat == 'feedburner' || $cat == 'site') {
        $feeds[] = $feed;
      }
      else {
        $feedburner_feeds[$feed['path']] = $feed;
      }
    }
  }

  $keys = array();
  $keepers = array();
  foreach ($feeds as $feed) {
    if (isset($feed['module']) && !module_exists($feed['module'])) {
      continue;
    }

    // Lookup alias
    if ($options['alias']) {
      $feed['alias'] = drupal_get_path_alias($feed['path']);
    }

    // Match regular expressions
    if ($options['match']) {
      $regex = '/' . preg_quote($options['match'], '/') . '/i';
      $feed['value'] = FALSE;
      foreach (array('path', 'alias', 'feedburner', $key) as $field) {
        if (isset($feed[$field]) && preg_match($regex, $feed[$field])) {
          $value = levenshtein($options['match'], $feed[$field]);
          if ($feed['value'] === FALSE || $value < $feed['value']) {
            $feed['value'] = $value;
          }
        }
      }
      if ($feed['value'] === FALSE) {
        continue;
      }
    }

    // Load FeedBurner Data
    if ($options['feedburner'] && isset($feedburner_feeds[$feed['path']])) {
      $feed = array_merge($feed, $feedburner_feeds[$feed['path']]);
    }

    // Merge information
    if (isset($keys[$feed['path']])) {
      $index = $keys[$feed['path']];
      $keepers[$index] = array_merge($feed, $keepers[$index]);
      continue;
    }

    // TODO: If $options['key'] is set, make associative array somehow
    $keys[$feed['path']] = count($keepers);
    $keepers[] = $feed;
  }

  $feeds = $keepers;

  // Sort by array key (custom sort)
  if ($options['sort']) {
    // My funkly little sort function that sorts 'blog' and 'blog1' ahead of 'blog/...' and recurses down each section of the path
    function _feedburner_temp_sort_path($a, $b) {
      $a = explode('/', $a, 2);
      $b = explode('/', $b, 2);
      //return (intval($a[0]) xor intval($b[0]) ? intval($a[0]) : ($result = strnatcasecmp($a[0], $b[0]) ? $result : _feedburner_temp_sort_path($a[1], $b[1])));
      if (intval($a[0]) xor intval($b[0])) {
        return intval($a[0]);
      }
      elseif ($result = strnatcasecmp($a[0], $b[0])) {
        return $result;
      }
      else {
        return _feedburner_temp_sort_path($a[1], $b[1]);
      }
    }

    // Need to pass the key variale inside these function declarations
    $GLOBALS['feedburner_sort_key'] = $options['key'] ? $options['key'] : 'path';
    function _feedburner_temp_sort_fields($a, $b) {
      if (isset($a['value']) && isset($b['value']) && $a['value'] != $b['value']) {
        return $a['value'] > $b['value'];
      }
      else {
        return _feedburner_temp_sort_path($a[$GLOBALS['feedburner_sort_key']], $b[$GLOBALS['feedburner_sort_key']]);
      }
    }

    usort($feeds, '_feedburner_temp_sort_fields');
    unset($GLOBALS['feedburner_sort_key']);
  }

  return $feeds;
}

function feedburner_autocomplete($cat) {
  $key = trim(_feedburner_get_path(5));
  $feeds = _feedburner_get_feeds($cat, array('match' => $key, 'feedburner' => FALSE));
  $feeds = array_slice($feeds, 0, 10);
  $keepers = array();
  foreach ($feeds as $feed) {
    if ($cat == 'feedburner') {
      $keepers[$feed['feedburner']] = $feed['feedburner'];
    }
    else {
      $keepers[$feed['path']] = ($feed['alias'] != $feed['path'] ? $feed['alias'] . ' (' . $feed['path'] . ')' : $feed['path']) . ($feed['description'] ? ' - ' . $feed['description'] : '');
    }
  }
  print drupal_to_js($keepers);
  exit();
}

/**
 * Performs a request to the FeedBurner APIs and returns the result.
 */
function _feedburner_request_api($function, $args = array()) {
  $request = new stdClass();

  // This function shouldn't be called if SimpleXML is not available.
  assert(_feedburner_can_api());

  $functions = array(
    'FindFeeds' => array('api' => 'management'),
    'GetFeed' => array('api' => 'management'),
    'AddFeed' => array('api' => 'management', 'method' => 'POST'),
    'ModifyFeed' => array('api' => 'management', 'method' => 'POST'),
    'DeleteFeed' => array('api' => 'management', 'method' => 'POST'),
    'ResyncFeed' => array('api' => 'management', 'method' => 'POST'),
    'GetFeedData' => array('api' => 'awareness'),
    'GetItemData' => array('api' => 'awareness'),
    'GetResyndicationData' => array('api' => 'awareness'),
  );

  if (!isset($functions[$function])) {
    $request->error = 'Function not found';
    return $request;
  }

  $request->function = $functions[$function];
  $request->url = 'http://api.feedburner.com/' . $request->function['api'] . '/1.0/' . $function;
  $request->method = isset($request->function['method']) ? $request->function['method'] : 'GET';
  $request->headers = array();
  $request->data = NULL;

  if ($request->function['api'] == 'management') {
    if (!isset($args['user']) && $auth = variable_get('feedburner_auth', FALSE)) {
      $request->headers['Authorization'] = 'Basic ' . $auth;
      unset($args['user']);
      unset($args['password']);
    }
    //$request->url = 'http://api.feedburner.com/management/1.0/' . $function;
  }
  elseif ($request->function['api'] == 'awareness') {
    //$request->url = 'http://api.feedburner.com/awareness/1.0/' . $function;
  }

  // Use secure request if PHP OpenSSL is available on any request
  if (extension_loaded('openssl')) {
    $request->url = str_replace('http', 'https', $request->url);
  }

  // Encode data and parameters depending on HTTP request method
  if ($request->method == 'POST') {
    $request->data = drupal_query_string_encode($args);
  }
  elseif ($request->method == 'GET') {
    $request->url .= '?' . drupal_query_string_encode($args);
  }

  $response = drupal_http_request($request->url, $request->headers, $request->method, $request->data);

  if (isset($response->data)) {
    // Convert XML data to an object using SimpleXML so it can be easily manipulated
    $response->data = simplexml_load_string($response->data);

    if (isset($response->data->err)) {
      // If an API error occurred, load the API error into the response error.
      $response->error = (string) $response->data->err['msg'];

      if (($response->error == 'Unknown user' || $response->error == 'Invalid password') && isset($request->headers['Authorization'])) {
        // Unset authentication token if the error involves an incorrect username or password (and not using manual authorization)
        //TODO Set Watchdog or error message
        //watchdog('feedburner', t('FeedBurner API Error: %error', array('%error' => $response->error)), WATCHDOG_ERROR);
        variable_set('feedburner_auth', FALSE);
      }
    }
  }

  return $response;
}

/**
 * Retreive details about a FeedBurner feed.
 */
function _feedburner_get_feed_features($uri, $cache = TRUE) {

  // If there is a cached result, return it instead of processing again
  $feed_cache = cache_get('feedburner_feeds_details');
  if (!$feed_cache) {
    $feed_cache = array();
  }
  elseif (isset($feed_cache->data[$uri]) && $cache) {
    return $feed_cache->data[$uri];
  }
  else {
    $feed_cache = $feed_cache->data;
  }

  $feed = new stdClass();
  $feed->id = 0;
  $feed->email = FALSE;
  $feed->feedflare = FALSE;
  $feed->awareness = FALSE;
  $feed->title = '';

  // Scrape details from the actual feed
  $request = drupal_http_request(_feedburner_construct_url($uri) . '?format=xml');

  if (isset($request->error)) {
    $feed->error = $request->error;
    return $feed;
  }
  elseif ($request->code == 200) {

    $features = array(
      'email' => array('regex' => '%</feedburner:emailServiceId>%i'),
      'id' => array('regex' => '%<feedburner:emailServiceId>(\d+)</feedburner:emailServiceId>%i', 'capture' => TRUE),
      'feedflare' => array('regex' => '%</feedburner:feedFlare>%i'),
      'awareness' => array('regex' => '%</feedburner:awareness>%i'),
      'title' => array('regex' => '%<title>(.*?)</title>%i', 'capture' => TRUE),
    );

    foreach ($features as $feature => $details) {
      if (!$feed->$feature && preg_match($details['regex'], $request->data, $matches)) {
        if (isset($details['capture'])) {
          $feed->$feature = $matches[1];
        }
        else {
          $feed->$feature = TRUE;
        }
      }
    }
  }

  //// Get details using the Feed Management API
  //if (_feedburner_can_api(TRUE)) {
  //  $request = _feedburner_request_api('GetFeed', array('uri' => $uri));
  //  var_export($request);
  //}

  // Cache results for quick retrieval
  $feed_cache[$uri] = $feed;
  cache_set('feedburner_feeds_details', $feed_cache, 'cache', CACHE_TEMPORARY);

  return $feed;
}
