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
    $form['term_match']['terms_list'] = [
      '#type' => 'fieldset',
      '#title' => t('Matching Terms'),
      '#description' => t('Please select a term.  You will be able to modify properties with this term type or value.'),
    ];
    $match = [
      'name' => $term_name,
    ];
    $terms = chado_generate_var('cvterm', $match, ['return_array' => TRUE]);
    $terms = chado_expand_var($terms, 'field', 'cvterm.definition');
    $num_terms = 0;
    $selected_term = '';

    // Let the user select from any matching terms. Sometimes there may be
    // more than one that match.
    foreach ($terms as $term) {
      // Save the user a click by setting the default value as 1 if there's
      // only one matching term.
      $default = FALSE;
      $attrs = [];
      if ($num_terms == 0 and count($terms) == 1) {
        $default = TRUE;
        $attrs = ['checked' => 'checked'];
        $selected_term = $term;
      }
      $term_element_name = 'term-' . $term->cvterm_id;
      $form['term_match']['terms_list'][$term_element_name] = [
        '#type' => 'radio',
        '#title' => $term->name,
        '#default_value' => $default,
        '#attributes' => $attrs,
        '#description' => '<b>Vocabulary:</b> ' . $term->cv_id->name . ' (' . $term->dbxref_id->db_id->name . ') ' . $term->cv_id->definition .
          '<br><b>Term: </b> ' . $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession . '.  ' .
          '<br><b>Definition:</b>  ' . $term->definition,
        '#ajax' => [
          'callback' => "tripal_curator_get_term_lookup_form_ajax_callback",
          'wrapper' => "curator-vocab-select-form",
          'effect' => 'fade',
          'method' => 'replace',
        ],
      ];

      if (array_key_exists('input', $form_state) and array_key_exists($term_element_name, $form_state['input']) and
        $form_state['input'][$term_element_name] == 'on') {
        $selected_term = $term;
      }
      $num_terms++;
    }

    if ($num_terms == 0) {
      $form['term_match']['terms_list']['none'] = [
        '#type' => 'item',
        '#markup' => '<i>' . t('There is no term that matches the entered text.') . '</i>',
      ];
    }

    if ($selected_term) {

      $props_by_type = new \tripal_curator\Chado_property();
      $props_by_value = new\tripal_curator\CValue();

      $props_by_type->set_cvtermprop_search($selected_term->cvterm_id);

      $props_by_value->set_cvalue_search($selected_term->cvterm_id);

      $properties_matching_type = $props_by_type->get_props();
      $properties_matching_type_count = $props_by_type->get_total();

      $properties_matching_cvalue = $props_by_value->get_properties();
      $properties_matching_cvalue_count = $props_by_value->get_total_count();

      $markup = "<h3>Properties with this type<h3/>";

      if ($properties_matching_type_count === 0) {

        $markup .= "<p>There are no properties utilizing this type ID</p>";
      }
      else {


        $link = l("Click to reassign properties with the selected CVterm for their type.  The below properties will be effected.", "/admin/tripal/extension/tripal_curator/edit_type_id/" . $selected_term->cvterm_id);

        $markup .= "<p>{$link}</p>";
      $rows = [];
      $headers = ["table", "count"];
        foreach ($properties_matching_type as $prop_table => $props){
          $rows[] = [$prop_table, count($props)];
        }

        $markup .= theme("table", ["rows" => $rows, "header" => $headers]);

      }

      $markup .= "<h3>Properties with this value cvterm<h3/>";

      if ($properties_matching_cvalue_count === 0) {
        $markup .= "<p>There are no properties utilizing this value cvterm ID</p>";
      }
      else {

        $link = l("Click to remap or split properties with the selectd CVterm value.  The below properties will be effected.", "/admin/tripal/extension/tripal_curator/edit_cvalue/cvalue_id/" . $selected_term->cvterm_id);

        $markup .= "<p>{$link}</p>";

        $rows = [];
        $headers = ["table", "count"];
        foreach ($properties_matching_cvalue as $prop_table => $props){
          $rows[] = [$prop_table, count($props)];
        }
        $markup .= theme("table", ["rows" => $rows, "header" => $headers]);
      }

      $form['term_match']['chosen_entities'] = [
        '#markup' => $markup,
      ];
    }
  }


  return $form;
}

function tripal_curator_get_term_lookup_form_ajax_callback($form, &$form_state) {
  return $form['term_match'];
}