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
