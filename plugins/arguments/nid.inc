<?php
// $Id: nid.inc,v 1.9 2009/10/12 12:42:53 sdboyer Exp $

/**
 * @file
 *
 * Plugin to provide an argument handler for a node id
 */

/**
 * Implementation of specially named hook_ctools_arguments().
 */
function ctools_nid_ctools_arguments() {
  return array(
    'title' => t("Node: ID"),
    'keyword' => 'node',
    'description' => t('Creates a node context from a node ID argument.'),
    'context' => 'ctools_argument_nid_context',
    'placeholder form' => array(
      '#type' => 'textfield',
      '#description' => t('Enter the node ID of a node for this argument'),
    ),
  );
}

/**
 * Discover if this argument gives us the node we crave.
 */
function ctools_argument_nid_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return ctools_context_create_empty('node');
  }

  // We can accept either a node object or a pure nid.
  if (is_object($arg)) {
    return ctools_context_create('node', $arg);
  }

  if (!is_numeric($arg)) {
    return FALSE;
  }

  $node = node_load($arg);
  if (!$node) {
    return FALSE;
  }

  return ctools_context_create('node', $node);
}

