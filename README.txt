// $Id$

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Requirements
 * Installation
 * Frequently Asked Questions (FAQ)
 * More Information


INTRODUCTION
------------

Current Maintainer: Dave Reid <dave@davereid.net>


REQUIREMENTS
------------

 * A valid Drupal installation
 * Optional: PHP 5 for more advanced features
 * Optional: Valid HTTP connections
 * Optional: A valid FeedBurner account


INSTALLATION
------------

See http://drupal.org/node/70151


FREQUENTLY ASKED QUESTIONS (FAQ)
--------------------------------

Q: Does this module work with feeds burnt with a Google FeedBurner account?
A: Yes it does! All you need to do is go to the module settings page
   (admin/settings/feedburner) and under the 'Advanced Settings' area, change
   the MyBrand domain to 'feedproxy.google.com'.

Q: How do I get the 'Comments Count' FeedFlare for my Drupal feed?
A: There are two options:
   1. Install and enable the Comment RSS module
      (http://drupal.org/project/commentrss). Then enable the 'Comments Count
      (self-hosted WordPress)' FeedFlare in the Optimize tab of the feed's
      FeedBurner settings at feedburner.com.
   2. In the FeedFlare settings  at feedburner.com, enter a the following url
      as a 'Personal FeedFlare':
      http://pathtoyourdrupalsite.com/feedburner/feedflare/comments


MORE INFORMATION
----------------

- To issue any bug reports, feature or support requests, see the module issue
  queue at http://drupal.org/project/issues/feedburner.

- For additional documentation, see the online module handbook at
  http://drupal.org/node.

- The Bryght team has put up a very nice guide and screencast about using the
  FeedBurner module at http://support.bryght.com/adminguide/feedburner.
