<?php
// $Id: term.inc,v 1.8 2009/10/17 22:58:25 sdboyer Exp $

/**
 * @file
 *
 * Plugin to provide a term context
 */

/**
 * Implementation of specially named hook_ctools_contexts().
 */
function ctools_term_ctools_contexts() {
  return array(
    'title' => t("Taxonomy term"),
    'description' => t('A single taxonomy term object.'),
    'context' => 'ctools_context_create_term',
    'settings form' => 'ctools_context_term_settings_form',
    'settings form validate' => 'ctools_context_term_settings_form_validate',
    'settings form submit' => 'ctools_context_term_settings_form_submit',
    'keyword' => 'term',
    'context name' => 'term',
    'convert list' => array(
      'tid' => t('Term ID'),
      'name' => t('Term name'),
      'vid' => t('Vocabulary ID'),
    ),
    'convert' => 'ctools_context_term_convert',
    'js' => array('misc/autocomplete.js'),
  );
}

/**
 * It's important to remember that $conf is optional here, because contexts
 * are not always created from the UI.
 */
function ctools_context_create_term($empty, $data = NULL, $conf = FALSE) {
  $context = new ctools_context('term');
  $context->plugin = 'term';

  if ($empty) {
    return $context;
  }

  if ($conf && isset($data['tid'])) {
    $data = taxonomy_term_load($data['tid']);
  }

  if (!empty($data)) {
    $context->data     = $data;
    $context->title    = $data->name;
    $context->argument = $data->tid;
    return $context;
  }
}

function ctools_context_term_settings_form($conf) {
  if (empty($conf)) {
    $conf = array(
      'vid' => '',
      'tid' => '',
      'term' => '',
    );
  }

  $form['vid'] = array(
    '#title' => t('Vocabulary'),
    '#type' => 'select',
    '#options' => array(),
    '#description' => t('Select the vocabulary for this form.'),
    '#id' => 'ctools-select-vid',
    '#default_value' => $conf['vid'],
  );

  $description = '';
  if (!empty($conf['tid'])) {
    $info = db_query('SELECT * FROM {taxonomy_term_data} WHERE tid = :tid', array(':tid' => $conf['tid']))->fetchObject();
    if ($info) {
      $description = ' ' . t('Currently set to @term. Enter another term if you wish to change the term.', array('@term' => $info->name));
    }
  }

  ctools_include('dependent');
  $options = array();

  // A note: Dependency works strangely on these forms as they have never been
  // updated to a more modern system so they are not individual forms of their
  // own like the content types.

  $form['taxonomy']['#tree'] = TRUE;

  foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
    $options[$vid] = $vocabulary->name;
    $form['taxonomy'][$vocabulary->vid] = array(
      '#type' => 'textfield',
      '#description' => t('Select a term from @vocabulary.', array('@vocabulary' => $vocabulary->name)) . $description,
      '#autocomplete_path' => 'taxonomy/autocomplete/' . $vocabulary->vid,
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array('ctools-select-vid' => array($vocabulary->vid)),
    );

  }

  $form['vid']['#options'] = $options;

  $form['tid'] = array(
    '#type' => 'value',
    '#value' => $conf['tid'],
  );

  $form['set_identifier'] = array(
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#title' => t('Reset identifier to term title'),
    '#description' => t('If checked, the identifier will be reset to the term name of the selected term.'),
  );

  return $form;
}

/**
 * Validate a term.
 */
function ctools_context_term_settings_form_validate($form, &$form_values, &$form_state) {
  // Validate the autocomplete
  $vid = $form_values['vid'];
  if (empty($form_values['tid']) && empty($form_values['taxonomy'][$vid])) {
    form_error($form['taxonomy'][$vid], t('You must select a term.'));
    return;
  }

  if (empty($form_values['taxonomy'][$vid])) {
    return;
  }

  $term = db_query('SELECT tid FROM {taxonomy_term_data} WHERE LOWER(name) = LOWER(:name) AND t.vid = :vid', array(':name' => $form_values['taxonomy'][$vid], ':vid' => $vid))->fetchObject();

  if (!$term) {
    form_error($form['taxonomy'][$vid], t('Invalid term selected.'));
  }
  else {
    form_set_value($form['tid'], $term->tid, $form_state);
  }
}

function ctools_context_term_settings_form_submit($form, &$form_values, &$form_state) {
  if ($form_values['set_identifier']) {
    $term = db_query('SELECT tid, name FROM {taxonomy_term_data} WHERE LOWER(tid) = :tid', array(':tid' => $form_values['tid']))->fetchObject();
    $form_state['values']['context']['identifier'] = $term->name;
  }

  // Don't let this be stored.
  unset($form_values['set_identifier']);
}

/**
 * Convert a context into a string.
 */
function ctools_context_term_convert($context, $type) {
  switch ($type) {
    case 'tid':
      return $context->data->tid;
    case 'name':
      return $context->data->name;
    case 'vid':
      return $context->data->vid;
  }
}