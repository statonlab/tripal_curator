<?php

/**
 * This form is for the CONVERSION and SPLITTING of values
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
function tripal_curator_assign_cvalue_form($form, &$form_state) {


  $markup = "<p>The Tripal Curator Cvalue mapper lets you easily batch assign CVterms to the value of your properties, where supported.  You can even split properties into multiple Properties!</p>";

  $markup .= '<p>This admin page lets you configure <b>CValues (CVterms for Values).</b>  In earlier versions of Chado, properties had a type with a cvterm, and a value that was free text.  If you wanted to use a CVterm for your value, you were out of luck.  No more!</p>
            <p>This form will let you assign CValues to your existing properties.  To begin, pick a search method.</p>
';
  $form['welcome'] = [
    '#markup' => $markup,
  ];

  //Step one: Search for *VALUE* entries
  $form['select_type'] = [
    '#title' => 'Cvalue Search Method',
    '#type' => 'select',
    '#options' => [
      '' => '-Please Select-',
      'text' => 'Text value',
      'existing' => 'Existing Cvalue',
      'blanks' => 'No Cvalue set',
    ],
    '#ajax' => [
      'callback' => 'tripal_curator_select_type_callback',
      'wrapper' => 'div1_wrapper',
      'effect' => 'fade',
    ],
  ];

  // Create the divs for AJAX rebuilding.
  //This div is for the SEARCH TYPE
  $form['div1'] = [
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#prefix' => '<div id="div1_wrapper">',
    '#suffix' => '</div>',
  ];

  //This div is for the SEARCH VALUES when a type is selected and requires further AJAX
  $form['div2'] = [
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#prefix' => '<div id="div2_wrapper">',
    '#suffix' => '</div>',
  ];


  $select_type = NULL;

  if (isset ($form_state['values']) && isset ($form_state['values']['select_type'])) {
    $select_type = $form_state['values']['select_type'];
  }

  if (!$select_type) {

    $form['div1']['kill'] = [
      '#markup' => '<p>Please select a Cvalue search method to continue.</p>',
    ];
    return $form;
  }

  $selection = tripal_curator_build_selection_instructions($select_type);

  $form['div1']['selection'] = $selection;


  if ($select_type === 'text' ){
    $form['div1']['search_value_text'] = [
      '#title' => t('Search the value text'),
      '#description' => '',
      '#type' => 'textfield',
      '#ajax' => [
        'callback' => 'tripal_curator_search_callback',
        'wrapper' => 'div2_wrapper',
        'effect' => 'fade',
      ],
    ];
    $form['div1']['search_buttons'] = [
      '#title' => t('Search'),
      '#type' => 'button',
      '#value' => 'search',
      '#ajax' => [
        'callback' => 'tripal_curator_search_callback',
        'wrapper' => 'div2_wrapper',
        'effect' => 'fade',
      ],
    ];


    $current_text = isset($form_state['values']['search_value_text']) ? $form_state['values']['search_value_text'] : null;

    if (!$current_text){
      $form['div2']['need_input'] = [
        '#markup' => 'Please enter text to search for property values'
      ];
      return $form;
    }

    $property_vals = new \tripal_curator\CValue();
    $property_vals->set_value_text($current_text);

    $total = $property_vals->get_total_count();
    $props = $property_vals->get_properties();


    $table = [];

      $table = '<h3>Properties matching ' .$current_text .  ' </h3>';

      $table .= "<p><strong>Total:  </strong>  {$total} </p>";

      foreach ($props as $table_name => $properties){
        $table_count = count($properties);
        $table .= "<p><strong>{$table_name}:  </strong>  {$table_count} </p>";
      }


      $link = l("Click to edit or split", "/admin/tripal/extension/tripal_curator/edit_cvalue/text/" . $current_text);

      $table .= "<strong>{$link}</strong>";


      $form['div2']['matching_property_values'] = [
      '#markup' => $table
    ];

  }


  if ($select_type === 'existing' ) {
$description = "";

    $form['div1']['search_cvterm'] = [
      '#title' => t('CVterm Lookup'),
      '#description' => $description,
      '#type' => 'textfield',
      '#autocomplete_path' => "admin/tripal/storage/chado/auto_name/cvterm/",
    ];


    $form['div1']['select_cvterm'] = [
      '#type' => 'button',
      '#value' => t('Lookup Term'),
      '#name' => 'select_cvterm',
      '#validate' => [],
      '#limit_validation_errors' => [],
      '#ajax' => [
        'callback' => "tripal_curator_cvalue_cvterm_callback",
        'wrapper' => "tripal_curator_search_callback",
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

  }


    return ($form);

  //Step two: Determine Cvalue


  //Step five: Convert


  return $form;

}

/**
 * Switchbox for the instructions based on the cvalue selection type.
 *
 * @param $type
 *
 * @return array
 */

function tripal_curator_build_selection_instructions($type) {

  $form = [];

  switch ($type) {
    case "text":
      $form = ['#markup' => 'Free text search  allows you to enter text matching hte property value.  As you type, a table of matching properties will be displayed.'];
      break;
    case "existing":
      $form = ['#markup' => 'Please choose an existing CVterm to search for.'];
      break;
    case "blanks":
      $form = tripal_curator_build_blanks_table();
      break;
  }

  return $form;

}


/*
 * Builds the blank cvalue table.
 */

function tripal_curator_build_blanks_table() {

  $rows = [];
  $header = [
    'Value',
    'Table',
    'Linked Entity',
    'CVterm name',
    'CV',
    'Number of Properties',
  ];

  $form = [];


  $property_object = new \tripal_curator\Chado_property();
  $property_object->build_blank_cvalues();
  $prop_tables = $property_object->get_props();

  foreach ($prop_tables as $table => $properties) {
    foreach ($properties as $property) {
      $type_id = $property->type_id;
      $value = $property->value;
      $prop_record_handle = $table . '_id';
      $prop_record_id = $property->$prop_record_handle;

      $cvterm = tripal_get_cvterm(['cvterm_id' => $type_id]);

      $linked_entity = l('coming soon', '1');
      $value = l($value, '/admin/tripal/extension/tripal_curator/edit_cvalue/text/' . $value);

      $rows[] = [
        'Value' => $value,
        'Table' => $table,
        'Linked Entity' => $linked_entity,
        'CVterm name' => $cvterm->name,
        'CV' => $cvterm->cv_id->name,
        'Number of Properties' => 'Coming Soon',
      ];
    }
  }

  $table = [
    'header' => $header,
    'rows' => $rows,
  ];

  $markup = '<h3>Properties with blank cvalues</h3>
       <p>Click on the "value" Column to <b>batch edit</b> properties with that value.</p>
        <p>Alternatively you can change properties one at a time using the <b>Linked Entity</b> column.</p>
        ';

  $markup .= theme('table', $table);

  $form = [
    '#type' => 'markup',
    '#markup' => $markup,
  ];
  return $form;
}

function tripal_curator_select_type_callback($form, &$form_state) {
  return $form['div1'];
}

function tripal_curator_search_callback($form, &$form_state) {
  return $form['div2'];
}