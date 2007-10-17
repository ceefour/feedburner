<?php
// $Id$

define('FEEDBURNER_API_MANAGEMENT', 'management');
define('FEEDBURNER_API_AWARENESS', 'awareness');

function _feedburner_api_query_encode($data, $keyprefix = '', $keypostfix = '') {
  assert(is_array($data));
  $vars = null;
  foreach ($data as $key => $value) {
    if (is_array($value)) {
      $vars .= $this->data_encode($value, $keyprefix . $key . $keypostfix . urlencode('['), urlencode(']'));
    }
    else {
      $vars .= $keyprefix . $key . $keypostfix . '=' . urlencode($value) . '&';
    }
  }
  return substr($vars, 0, -1);
}

function _feedburner_api_request($function, $args = array()) {
  $requst = new stdClass();
  if (!_feedburner_can_api()) {
    $request->error = 'SimpleXML is not enabled and is required for use of the FeedBurner API';
    return $request;
  }

  switch ($function) {
    case 'FindFeeds':
    case 'GetFeed':
    case 'AddFeed':
    case 'ModifyFeed':
    case 'DeleteFeed':
    case 'ResyncFeed':
      $api = FEEDBURNER_API_MANAGEMENT;
      break;
    case 'GetFeedData':
      $api = FEEDBURNER_API_AWARENESS;
      break;
    default:
      $request->error = 'Function not found';
      return $request;
  }

  /*$functions = array(
    'FindFeeds' => FEEDBURENR_API_MANAGEMENT,
    'GetFeed' => FEEDBURENR_API_MANAGEMENT,
    'AddFeed' => FEEDBURENR_API_MANAGEMENT,
    'ModifyFeed' => FEEDBURENR_API_MANAGEMENT,
    'DeleteFeed' => FEEDBURENR_API_MANAGEMENT,
    'ResyncFeed' => FEEDBURENR_API_MANAGEMENT,
    'GetFeedData' => FEEDBURNER_API_AWARENESS,
  );
  if (!isset($functions[$function])) {
    return false;
  }

  $api = $functions[$function];*/
  $headers = array();

  // Insert authorization tokens/header for the management API
  $manual_auth = isset($args['user']) && isset($args['password']);
  if ($api == FEEDBURNER_API_MANAGEMENT && !$manual_auth) {
    $auth = variable_get('feedburner_auth', null);
    if (empty($auth)) {
      $request->error = 'Authorization required';
      return $request;
    }
    else {
      $headers['Authorization'] = 'Basic '. $auth;
      unset($args['user']);
      unset($args['password']);
    }
  }

  // Construct the API url
  $url = 'http://api.feedburner.com/'. $api .'/1.0/'. $function .'?'. _feedburner_api_query_encode($args);

  $request = drupal_http_request($url, $headers);
  if (isset($request->data)) {

    // Convert XML data to an object so it can be easily manipulated in PHP using SimpleXML
    //_feedburner_include('xml');
    //$request->data = XML_unserialize($request->data);
    $request->data = simplexml_load_string($request->data);

    // If the API errored, load the api error string into the request error string
    if (isset($request->data->err)) {
      $request->error = (string) $request->data->err['msg'];

      // Unset authentication token if the error involves an incorrect username or password (and not using manual authorization)
      if (!$manual_auth && ($request->error == 'Unknown user' || $request->error == 'Invalid password')) {
        variable_set('feedburner_auth', null);
      }
    }
  }

  return $request;
}

function _feedburner_api_verify_account(&$account) {
  $account['auth'] = null;
  $api_request = _feedburner_api_request('FindFeeds', $account);
  if (isset($api_request->error)) {
    $account['error'] = $api_request->error;
  }
  else if (isset($api_request->data)) {
    $account['auth'] = base64_encode($account['user'] .':'. $account['password']);
  }
  else {
    $account['error'] = 'Unknown error during FeedBurner account verification.';
  }
}
