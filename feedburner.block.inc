<?php
// $Id$

//require_once('./'. drupal_get_path('module', 'feedburner') .'/feedburner.admin.inc');

/**
 * Implementation of hook_block().
 */
function _feedburner_block($op, $delta, $edit) {
  switch ($op) {
    case 'list':
      $blocks[0]['info'] = t('FeedBurner FeedFlare');
      $blocks[1]['info'] = t('FeedBurner E-mail Subscription');
      $blocks[2]['info'] = t('FeedBurner Feed Subscribers');
      return $blocks;
    case 'configure':
      switch ($delta) {
        case 1:
          $result = db_query("SELECT DISTINCT fb_uri, details FROM {feedburner_feeds} GROUP BY fb_uri");
          $feeds = array();
          while ($feed = db_fetch_object($result)) {
            $feed->details = unserialize($feed->details);
            if (isset($feed->details['id'])) {
              $feeds[$feed->details['id']] = $feed->fb_uri;
            }
          }

          $form['feedburner_block_email'] = array(
            '#type' => 'select',
            '#title' => t('Selected feed'),
            '#default_value' => variable_get('feedburner_block_email', null),
            '#options' => $feeds,
          );
          break;
        case 2:

          break;
      }
      return $form;
    case 'save':
      switch ($delta) {
        case 1:
          variable_set('feedburner_block_email', intval($edit['feedburner_block_email']));
          break;
      }
      break;
    case 'view':
      switch ($delta) {
        case 0:
          if (arg(0) == 'node' && is_numeric(arg(1))) {
            $node = node_load(arg(1));
            //$feedflare = _feedburner_get_feedflare($node->nid, $node->type);
            //if ($feedflare != false) {
              $block['subject'] = t('FeedFlare');
              //$block['content'] = $feedflare;
            //}
          }
          break;
        case 1:
          $id = variable_get('feedburner_block_email', 0);
          if ($id != 0) {
            $block['subject'] = t('Subscribe via Email');
            $form = '<form action="http://www.feedburner.com/fb/a/emailverify" method="post" target="popupwindow" onsubmit="window.open(\'http://www.feedburner.com\', \'popupwindow\', \'scrollbars=yes,width=550,height=520\');return true">';
            $form .= '<p>Enter your email address:</p>';
            $form .= '<p><input type="text" style="width:140px" name="email"/></p>';
            $form .= '<input type="hidden" value="http://feeds.feedburner.com/~e?ffid='. variable_get('feedburner_block_email', 0) .'" name="url"/>';
            $form .= '<input type="submit" value="Subscribe" />';
            $form .= '<p>Delivered by <a href="http://www.feedburner.com" target="_blank">FeedBurner</a></p>';
            $form .= '</form>';
            $block['content'] = $form;
          }
          break;
        case 2:
          $block['subject'] = t('Feed Stats');
          $block['content'] = '';
          break;
      } //switch
      return $block;
  } //switch
}
