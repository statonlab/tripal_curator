<?php

/**
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */

function tripal_curator_form($form, &$form_state) {

  $markup = "<h3>Tripal Chado Property Curations</h3>
    <p>Welcome to the Tripal Property Curator!</p><p>
    For more information on how to use this page, please see the github repo.
</p>
";

  $form['welcome'] = [
    '#markup' => $markup,
  ];


  $form['term_match'] = [
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div id = "curator-vocab-select-form">',
    '#suffix' => '</div>',
  ];

  $description = t('Search for a term to modify properties utilizing it as a type or value');
  $form['term_match']['term_name'] = [
    '#title' => t('CVterm Lookup'),
    '#description' => $description,
    '#type' => 'textfield',
    '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/",
  ];

  $form['term_match']['select_cvterm'] = [
    '#type' => 'button',
    '#value' => t('Lookup Term'),
    '#name' => 'select_cvterm',
    '#validate' => [],
    '#limit_validation_errors' => [],
    '#ajax' => [
      'callback' => "tripal_curator_get_term_lookup_form_ajax_callback",
      'wrapper' => "curator-vocab-select-form",
      'effect' => 'fade',
      'method' => 'replace',
    ],
  ];

  // If the term has been provided by the user then we want to search for
  // matching terms in the database and let them select among any matches.
  $term_name = '';
  if (array_key_exists('values', $form_state) and array_key_exists('term_name', $form_state['values'])) {
    $term_name = array_key_exists('values', $form_state) ? $form_state['values']['term_name'] : '';
  }
  if (array_key_exists('input', $form_state) and array_key_exists('term_name', $form_state['input'])) {
    $term_name = array_key_exists('input', $form_state) ? $form_state['input']['term_name'] : '';
  }

  if ($term_name) {
    $submit_disabled = TRUE;
    $form['term_match']['terms_list'] = array(
      '#type' => 'fieldset',
      '#title' => t('Matching Terms'),
      '#description' => t('Please select a term.  You will be able to modify properties with this term type or value.')
    );
    $match = array(
      'name' => $term_name,
    );
    $terms = chado_generate_var('cvterm', $match, array('return_array' => TRUE));
    $terms = chado_expand_var($terms, 'field', 'cvterm.definition');
    $num_terms = 0;
    $selected_term = '';

    // Let the user select from any matching terms. Sometimes there may be
    // more than one that match.
    foreach ($terms as $term) {
      // Save the user a click by setting the default value as 1 if there's
      // only one matching term.
      $default = FALSE;
      $attrs = array();
      if ($num_terms == 0 and count($terms) == 1) {
        $default = TRUE;
        $attrs = array('checked' => 'checked');
      }
      $term_element_name = 'term-' . $term->cvterm_id;
      $form['term_match']['terms_list'][$term_element_name] = array(
        '#type' => 'radio',
        '#title' =>  $term->name,
        '#default_value' => $default,
        '#attributes' => $attrs,
        '#description' => '<b>Vocabulary:</b> ' . $term->cv_id->name . ' (' . $term->dbxref_id->db_id->name . ') ' . $term->cv_id->definition .
          '<br><b>Term: </b> ' . $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession . '.  ' .
          '<br><b>Definition:</b>  ' . $term->definition,
        '#ajax' => array(
          'callback' => "tripal_curator_get_term_lookup_form_ajax_callback",
          'wrapper' => "curator-vocab-select-form",
          'effect' => 'fade',
          'method' => 'replace'
        ),
      );

      if (array_key_exists('values', $form_state) and array_key_exists($term_element_name, $form_state['values']) and
        $form_state['values'][$term_element_name] == 1) {
        $selected_term = $term;
      }
      $num_terms++;
    }


    if ($num_terms == 0) {
      $form['term_match']['terms_list']['none'] = array(
        '#type' => 'item',
        '#markup' => '<i>' . t('There is no term that matches the entered text.') . '</i>'
      );
    }

    if ($selected_term){

     $property_converter =  new \tripal_curator\Chado_property();

     $property_converter->set_cvtermprop_search($selected_term->cvterm_id);

      $form['chosen_entities'] = [
        '#markup' => ''
      ];

      $form['choose_remap'] = [
        '#type' => 'fieldset',
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#prefix' => '<div id = "curator-choose-field">',
        '#suffix' => '</div>',
      ];

      $markup = '<h3>Remapping Properties</h3>  <p>This section will allow you to remap your Chado properties.</p>';
      $form['choose_remap']['instructions']= [
        '#markup' => $markup
      ];



    }


  }




  return $form;
}

function tripal_curator_get_term_lookup_form_ajax_callback($form, &$form_state) {
  return $form['term_match'];
}