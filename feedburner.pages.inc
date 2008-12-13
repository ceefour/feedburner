<?php
// $Id$

function feedburner_feedflare($feedflare) {
  $output = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  $output .= "<!DOCTYPE FeedFlareUnit SYSTEM \"FeedFlareUnit-1.0.dtd\">\n";

  switch ($feedflare) {
    case 'comments':
      if (isset($_GET['nid']) && is_numeric($_GET['nid'])) {
        $node = node_load($_GET['nid']);
        if (isset($node->comment) && $node->comment != COMMENT_NODE_DISABLED) {
          $output .= "<FeedFlare>\n";
          $output .= "  <Text>". format_plural($node->comment_count, '@count comment', '@count comments') ."</Text>\n";
          $output .= "  <Link href=\"". url('node/'. $node->nid, array('fragment' => 'comments', 'absolute' => TRUE)) ."\" />\n";
          $output .= "</FeedFlare>\n";
        }
      }
      else {
        $output .= "<FeedFlareUnit>\n";
        $output .= "  <Catalog>\n";
        $output .= "    <Title>Drupal comment count</Title>\n";
        $output .= "    <Description>Returns the number of comments for a Drupal node</Description>\n";
        $output .= "    <Link href=\"http://drupal.org/project/feedburner\" />\n";
        $output .= "    <Author>Dave Reid</Author>\n";
        $output .= "  </Catalog>\n";
        $output .= "  <DynamicFlare href=\"". url('feedburner/feedflare/comments', array('absolute' => TRUE, 'query' => "nid=\${fn:substring-before(a:id, ' ')}")) ."\" />\n";
        $output .= "</FeedFlareUnit>\n";
      }
  }

  drupal_set_header('Content-Type: application/xml; charset=utf-8');
  print $output;
}

/**
 * Auto-completion callback.
 *
 * @todo REWRITE AND REVISE!
 */
function feedburner_autocomplete($category) {
  module_load_include('inc', 'feedburner', 'feedburner.admin');
  $key = _feedburner_get_path(6);
  $options = array('match' => $key, 'limit' => 10, 'feedburner' => TRUE);

  if ($category == 'feedburner') {
    $options += array('key' => 'feedburner', 'fields' => 'feedburner');
  }
  else {
    $options += array('key' => 'path', 'alias' => TRUE);
  }

  // Fetch list of feeds
  $feeds = _feedburner_get_feed_list($category, $options);

  if ($category != 'feedburner') {
    foreach ($feeds as $key => $feed) {
      $description = '';
      if ($feed['alias'] != $feed['path']) {
        $description .= $feed['alias'] .' ('. $feed['path'] .')';
      }
      else {
        $description .= $feed['path'];
      }
      if (isset($feed['description'])) {
        $description .= ' - '. $feed['description'];
      }
      $feeds[$key] = $description;
    }
  }

  drupal_json($feeds);

  /*$feeds = _feedburner_get_feed_list($cat, array('match' => $key, 'feedburner' => FALSE));
  $feeds = array_slice($feeds, 0, 10);
  $keepers = array();
  foreach ($feeds as $feed) {
    if ($cat == 'feedburner') {
      $keepers[$feed['feedburner']] = $feed['feedburner'];
    }
    else {
      $keepers[$feed['path']] = ($feed['alias'] != $feed['path'] ? $feed['alias'] .' ('. $feed['path'] .')' : $feed['path']) . ($feed['description'] ? ' - '. $feed['description'] : '');
    }
  }

  drupal_json($keepers);*/
}
