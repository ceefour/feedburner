<?php
// $Id$

require_once('./'. drupal_get_path('module', 'feedburner') .'/feedburner.admin.inc');

/**
 * Implementation of hook_user().
 *
 * Allows the user to set FeedBurner redirection for his/her own blog's feed.
 */
function _feedburner_user($op, &$edit, &$account, $category = null) {
  if (module_exists('blog') && user_access('edit own blog') && variable_get('feedburner_blogs', false)) {
    $user_blog = 'blog/'. $account->uid .'/feed';
    switch ($op) {
      case 'form':
        if ($category == 'account') {
          $feed = _feedburner_load($user_blog);
          $form['feedburner'] = array(
            '#type' => 'fieldset',
            '#title' => t('Blog FeedBurner Redirection'),
            '#weight' => 1,
            '#collapsible' => true,
            '#description' => t('Requests for your blog\'s feed (%url) will be redirected to this FeedBurner feed. Leave this blank to disable redirection.', array('%url' => url($user_blog, null, null, true))),
          );
          $form['feedburner']['fb_uri'] = array(
            '#type' => 'textfield',
            '#maxlength' => 100,
            '#size' => 30,
            '#default_value' => $feed['fb_uri'],
            '#field_prefix' => _feedburner_url(),
            '#description' => t('This URI is case-sensitive and alphanumeric.'),
            '#validate' => array('_feedburner_build_edit_validate' => array('fb_uri')),
          );
          return $form;
        }
        break;
      case 'update':
        if (_feedburner_save($user_blog, ($edit['status'] == 0 ? null : $edit['fb_uri'])) == true) {
          _feedburner_verify_feed($edit['fb_uri']);
        }
        unset($edit['fb_uri']);
        break;
      case 'delete':
        _feedburner_save($user_blog);
        break;
    } // switch
  }
}