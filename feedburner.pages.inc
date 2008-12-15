<?php
// $Id$

/**
 * @file
 * Page callback file for the feedburner module.
 */

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

function feedburner_block_feedflare(&$block) {
  if (arg(0) == 'node' && is_numeric(arg(1)) && arg(2) == NULL) {
    $node = node_load(arg(1));
    if ($feedflare = _feedburner_get_feedflare_script($node)) {
      $block['subject'] = t('FeedFlare');
      $block['content'] = $feedflare;
    }
  }
}

function feedburner_block_email(&$block) {
  if ($id = feedburner_var('block_email_id')) {
    // TODO: Convert to Form API? or better containing divs?
    $block['subject'] = t('Get E-mail Updates');
    $block['content'] = '<form action="http://www.feedburner.com/fb/a/emailverify" method="post" target="popupwindow" onsubmit="window.open(\'http://www.feedburner.com\', \'popupwindow\', \'scrollbars=yes,width=550,height=520\');return true;" id="feedburner-block-email-form"><div class="container-inline" id="feedburner-block-email">';
    $block['content'] .= '<div class="form-item">';
    $block['content'] .= '<label for="feedburner-block-email-form-email">Enter your email address:</label>';
    $block['content'] .= '<input type="text" size="15" name="email" id="feedburner-block-email-form-email" />';
    $block['content'] .= '</div>';
    $block['content'] .= '<input type="hidden" value="http://feeds.feedburner.com/~e?ffid='. $id .'" name="url"/>';
    $block['content'] .= '<input type="submit" value="Subscribe" class="form-submit" />';
    $block['content'] .= '<p>Delivered by <a href="http://www.feedburner.com">FeedBurner</a></p>';
    $block['content'] .= '</div></form>';
  }
}

function feedburner_block_feedcount(&$block) {
  if ($feed = feedburner_var('block_feedcount_feed')) {
    $block['content'] = '<a href="'. _feedburner_construct_url($feed) .'"><img src="http://feeds.feedburner.com/~fc/'. $feed .'?bg=99CCFF&amp;fg=444444&amp;anim=0" height="26" width="88" style="border:0" alt="" /></a>';
  }
}

function feedburner_block_feeds(&$block) {
  module_load_include('inc', 'feedburner', 'feedburner.admin');

  $items = array();
  $query = db_query("SELECT path, feedburner FROM {feedburner}");
  while ($feed = db_fetch_object($query)) {
    $features = _feedburner_get_feed_features($feed->feedburner);
    $items[] = theme('image', 'misc/feed.png') .' '. l(($features->title ? $features->title : $feed->path), _feedburner_construct_url($feed->feedburner));
  }

  if (!empty($items)) {
    $block['subject'] = t('@sitename feeds', array('@sitename' => variable_get('site_name', 'Drupal')));
    $block['content'] = theme('item_list', $items);
  }
}
