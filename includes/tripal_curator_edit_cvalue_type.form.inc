<?php

/**
 * This form is for the actual work of Curator: changing properties.
 * It is for a cvalue page.
 * If I'm smart, it should be able to handle blank, uncreated, text cvalues as
 * well.
 *
 */

/**
 *
 * @param $form
 * @param $form_state
 * @param $type - 'text' or 'cvalue'
 * @param $value  the text value or the cvalue_id
 *
 * @return mixed
 */
function tripal_curator_edit_cvalue_form($form, &$form_state, $type, $value) {


  $cval = new \tripal_curator\CValue();

  $instruction_markup = NULL;

  if ($type == "text") {
    $form['source_text'] = [
      '#type' => 'value',
      '#value' => $value,
    ];
    $cval->set_value_text($value);

    $instruction_markup = "<p><strong>Property Selection Type</strong>: Text</p>
                            <p>The properties listed below all share the same text-based value.</p>";
    $instruction_markup .= "
             <p><strong>Value</strong>: {$value}</p>
        ";
  }

  if ($type == "cvalue_id") {

    $form['source_cvalue_id'] = [
      '#type' => 'value',
      '#value' => $value,
    ];

    $cvterm = tripal_get_cvterm([
      'cvterm_id' => $value,
    ]);

    $cvterm_name = $cvterm->name;
    $cvterm_definition = 'Undefined.';


    if (isset($cvterm->definition)) {
      $cvterm_definition = $cvterm->definition;
    }

    $cval->set_cvalue_search($value);

    $instruction_markup = "<p><strong>Property Selection Type</strong>: Cvalue</p>
                            The properties listed below all share the same CVterm value (Cvalue).";

    $instruction_markup = "<p><strong>Term Name</strong>: {$cvterm_name}</p>";
    $instruction_markup = "<p><strong>Term Definition</strong>: {cvterm_definition}</p>";

  }


  $properties = $cval->get_properties();

  $total_count = $cval->get_total_count();


  $instruction_markup .= '<p><b>Total Properties: </b>' . $total_count . '</p>';


  $form['Value Information'] = [
    '#type' => 'fieldset',
  ];

  $form['Value Information']['input'] = [
    '#markup' => $instruction_markup,
  ];

  $form['Property Table'] = [
    '#type' => 'fieldset',
  ];

  $table = tripal_curator_edit_cvalue_provide_property_table($properties);



  //TODO does it make sense to push in choices based on the $type?
  $action_choices = [
    '' => '-Please Select-',
    'split' => "Split Properties",
    "set_cvalue" => "Set Property Value CVterm",
  ];


$default = '';

  if (isset($form_state['values']) && isset($form_state['values']['action'])){
    $default = $form_state['values']['action'];

  }

  $form['action'] = [
    '#type' => 'select',
    '#options' => $action_choices,
    '#default' => $default,
    '#ajax' => [
      'callback' => "tripal_curator_action_callback",
      'wrapper' => "action_type_wrapper",
      'effect' => 'fade',
      'method' => 'replace',
    ],
  ];

  $form['action_type_wrapper'] = [
    '#type' => 'fieldset',
    '#prefix' => '<div id="action_type_wrapper">',
    '#suffix' => '</div>',
  ];
  $action = NULL;
  if (isset($form_state['values']) && isset($form_state['values']['action'])) {
    $action = $form_state['values']['action'];
  }

  $form['action_type_wrapper']['instructions'] = [
    '#markup' => 'Please select an action above to continue',
  ];


  if ($action == "split") {
    $form['action_type_wrapper']['instructions'] = [
      '#markup' => '<h3>Splitting Properties</h3>
<p>You can split the above properties into <strong>multiple properties</strong>.  This is useful for properties that include multiple information values: for example, a treatment property that describes soil, ozone, and temperature treatments.</p>
<p>To split your properties, you must provide a set of output properties.</p> ',
    ];

  }

  if ($action == "set_cvalue") {
    $form['action_type_wrapper']['instructions'] = [
      '#markup' => '<h3>Setting a Value CVterm</h3>
<p>You can re-annotate the above properties to utilize a CVterm for the value (cvalue).</p>
<p>To proceed, please select a <strong>CVterm</strong> to assign the property values to.</p> ',
    ];





    //TODO: this is mega not-DRY.
    //Refactor it!


    $description = t('Search for a destination term to set the CVterm value to.');

    $form['action_type_wrapper']['search_cvterm'] = [
      '#title' => t('CVterm Lookup'),
      '#description' => $description,
      '#type' => 'textfield',
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/",
    ];


    $form['action_type_wrapper']['select_cvterm'] = [
      '#type' => 'button',
      '#value' => t('Lookup Term'),
      '#name' => 'select_cvterm',
      '#validate' => [],
      '#limit_validation_errors' => [],
      '#ajax' => [
        'callback' => "tripal_curator_action_callback",
        'wrapper' => "action_type_wrapper",
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
      $form['action_type_wrapper']['terms_list'] = [
        '#type' => 'fieldset',
        '#title' => t('Matching Terms'),
        '#description' => t('Please select a destination term.  All properties above will be converted to this property type.'),
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
        $form['action_type_wrapper']['terms_list'][$term_element_name] = [
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



    $form['action_type_wrapper']['overwrite_value'] = [
      '#type' => 'radios',
      '#title' => "Overwrite Text Value with Property Name",
      '#description' => "You can choose to overwrite the text value of the property with the name of your chosen value cvterm.",
      '#options' => ['Overwrite', 'Keep existing'],
    ];

  }

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => 'submit'
  ];

  return $form;


  ///////

  $chado_property->set_cvtermprop_search($type_id);

  $total_count = $chado_property->get_total();


  $info_markup = '<p><b>Name: </b>' . $cvterm_name . '</p>';
  $info_markup .= '<p><b>Definition: </b>' . $cvterm_definition . '</p>';

  $form['Source Type'] = [
    '#type' => 'fieldset',
    '#title' => 'Source CVterm Type',
    '#description' => 'This is the selected property type.',
  ];


  $form['Source Type']['info'] = [
    '#markup' => $info_markup,
  ];


  if ($total_count === 0) {

    $form['Source Type']['kill'] = [
      '#markup' => "No properties utilize this CVterm as their type!",
    ];
    return ($form);

  }

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
      '#description' => t('Please select a destination term.  All properties above will be converted to this property type.'),
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


  $default_c_type = 'all';


  $form['curation_type'] = [
    '#title' => 'Curation Type',
    '#type' => 'radios',
    '#description' => 'Curate all properties, or just specific entity types?',
    '#options' => [
      'all' => 'All Property Tables',
      'select' => 'Select Property Tables',
    ],
    '#default_value' => $default_c_type,
    '#ajax' => [
      'callback' => 'tripal_curator_type_callback',
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


  if ($curation_scope == $default_c_type) {
    //We're converting all properties

    $submit_message = '<p>Submitting this form will convert <b>ALL properties</b> of this type in <b>ALL</b> Chado property tables to the selected CVterm.  This cannot be undone.</p>';
    $form['div1']['submit_message'] = [
      '#markup' => $submit_message,
    ];

  }

  if ($curation_scope == 'select') {
    //We're converting select property tables
    $property_tables = [];

    foreach ($properties as $table_name => $property) {
      $count = $chado_property->get_table_count($table_name);

      $property_tables[$table_name] = [
        'name' => $table_name,
        'count' => $count,
      ];
    }


    $header = [
      'name' => t('Property Table Name'),
      'count' => t('Number of Entries'),
    ];

    $form['div1']['property_table_select'] = [
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $property_tables,
      '#multiple' => TRUE,
    ];

  }


  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Convert Property Type'),
  ];
  return $form;
}


function tripal_curator_action_callback($form, &$form_state) {
  return $form['action_type_wrapper'];
}

function tripal_curator_edit_cvalue_provide_property_table($properties){

  $count_total = 0;
  $rows = [];
  $headers = ["table", "property value", "property cvterm"];

  foreach ($properties as $table => $props){
    $count = 0;
    foreach ($props as $prop){

     $cvalue_id =  $prop->cvalue_id;
     $cvterm = tripal_get_cvterm(['cvterm_id' => $cvalue_id]);

      $rows[] = [$table, $prop->value, $cvterm->name];

      $count_total++;
    }
  }

  if ($count_total > 100){

  }

  $table = theme('table', ['rows' => $rows, 'header' => $headers]);

  return($table);

}
