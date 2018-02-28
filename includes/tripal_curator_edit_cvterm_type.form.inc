<?php

/**
 * This form is for the actual work of Curator: changing properties.
 *
 */

/**
 *
 * @param $form
 * @param $form_state
 * @param $type_id
 *
 * @return mixed
 */
function tripal_curator_edit_cvterm_type_form($form, &$form_state, $type_id) {


  ### Display type_id info

  //I dont end up doing a whole lot with this class... maybe dont need
  $cvterm = new \tripal_curator\Cvterm();
  $cvterm->set_id($type_id);
  $cvterm_record = $cvterm->get_full();

  //instead, it would be

  // $cvterm = tripal_get_cvterm(array(
  //'cvterm_id' => $this->cvterm_id,
  //  ));

  $cvterm_name = $cvterm_record->name;
  $cvterm_definition = $cvterm_record->definition;


  $chado_property = new \tripal_curator\Chado_property();
  $chado_property->set_cvtermprop_search($type_id);

  $total_count = $chado_property->get_total();
  $properties = $chado_property->get_props();


  $info_markup = '<p><b>Name: </b>' . $cvterm_name . '</p>';
  $info_markup .= '<p><b>Definition: </b>' . $cvterm_definition . '</p>';
  $info_markup .= '<p><b>Total uses: </b>' . $total_count . '</p>';

  $form['Source Type'] = [
    '#type' => 'fieldset',
    '#title' => 'Source CVterm Type',
    '#description' => 'This is the selected property type.'
  ];

  $form['Source Type']['info'] = [
    '#markup' => $info_markup,
  ];

  $form['destination_type'] = [
    '#type' => 'fieldset',
    '#title' => 'Destination CVterm Type',
    '#description' => 'Please specify a CVterm to convert your properties to.',
    '#prefix' => '<div id = "curator-vocab-select-form">',
    '#suffix' => '</div>',
  ];

  $description = t('Search for a destination term to convert this property to.');

  $form['destination_type']['search_cvterm'] = [
    '#title' => t('CVterm Lookup'),
    '#description' => $description,
    '#type' => 'textfield',
    '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/",
  ];


  $form['destination_type']['select_cvterm'] = [
    '#type' => 'button',
    '#value' => t('Lookup Term'),
    '#name' => 'select_cvterm',
    '#validate' => [],
    '#limit_validation_errors' => [],
    '#ajax' => [
      'callback' => "tripal_curator_destination_property_callback",
      'wrapper' => "curator-vocab-select-form",
      'effect' => 'fade',
      'method' => 'replace',
    ],
  ];


  $term_name = '';

  if (array_key_exists('input', $form_state) and array_key_exists('search_cvterm', $form_state['input'])) {
    $term_name = array_key_exists('input', $form_state) ? $form_state['input']['search_cvterm'] : '';
  }


  if ($term_name) {
    $submit_disabled = TRUE;
    $form['destination_type']['terms_list'] = [
      '#type' => 'fieldset',
      '#title' => t('Matching Terms'),
      '#description' => t('Please select a destination term.  All properties above will be converted to this property type.')
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
      }
      $term_element_name = 'term-' . $term->cvterm_id;
      $form['destination_type']['terms_list'][$term_element_name] = [
        '#type' => 'radio',
        '#title' => $term->name,
        '#default_value' => $default,
        '#attributes' => $attrs,
        '#description' => '<b>Vocabulary:</b> ' . $term->cv_id->name . ' (' . $term->dbxref_id->db_id->name . ') ' . $term->cv_id->definition .
          '<br><b>Term: </b> ' . $term->dbxref_id->db_id->name . ':' . $term->dbxref_id->accession . '.  ' .
          '<br><b>Definition:</b>  ' . $term->definition,
      ];

      $num_terms++;
    }
  }


  $default_c_type = 'All Property Tables';


  $form['curation_type'] = [
    '#title' => 'Curation Type',
    '#type' => 'radios',
    '#description' => 'Curate all properties, or just specific entity types?',
    '#options' => ['All Property Tables', 'Select Property Tables'],
    '#default_value' => 'All Property Tables',
    '#ajax' => [
      'callback' => $default_c_type,
      'wrapper' => 'div1_wrapper',
      'effect' => 'fade',
    ],
  ];

  // Create the divs for AJAX rebuilding.  One for the destination bundle select, one for the transaction overview
  $form['div1'] = [
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#prefix' => '<div id="div1_wrapper">',
    '#suffix' => '</div>',
  ];


  $curation_scope = isset($form_state['values']['curation_type']) ? $form_state['values']['curation_type'] : $default_c_type;


  if ($curation_scope == $default_c_type){
    //We're converting all properties

    $submit_message = '<p>Submitting this form will convert <b>ALL properties</b> of this type in <b>ALL</b> Chado property tables to teh selected CVterm.  This cannot be undone.</p>';
    $form['div1']['submit_message'] = [
      '#markup' => $submit_message
    ];

  }


  if ($curation_scope == 'Select Property Tables'){
    //We're converting select property tables
    $property_tables = [];

    foreach ($properties as $table_name => $property) {

      $property_tables[] = $table_name;

    }
  }



  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Convert Property Type')
  );
  return $form;
}


function tripal_curator_edit_cvterm_type_form_submit($form, &$form_state) {

}

function tripal_curator_type_callback($form, &$form_state){

  return $form['div1'];
}

function tripal_curator_destination_property_callback($form, &$form_state){
  return $form['destination_type'];
}
