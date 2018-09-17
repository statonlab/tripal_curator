<?php

/**
 *
 */
function tripal_curator_split_prop_form($form, &$form_state, $cvterm_id) {

  $term_object = new \tripal_curator\Cvterm();
  $box = provide_summary_box($cvterm_id, $term_object);
  $split_link = l('Click here to reassign this property instead.', 'admin/tripal/extension/tripal_curator/split/' . $cvterm_id);


  $markup = '<p>This form allows you to create a new child term of a different type from a subset of the property value.  For example, a term of type: <i>animal</i>, value: <i>red dog</i> could be split into two terms: animal:dog, color:red.</p>
 ' . $split_link;

  $form['intro'] = ['#markup' => $markup];

  $form['box'] = $box;

  $chado_property = new \tripal_curator\Chado_property();
  $chado_property->set_cvtermprop_search($cvterm_id);

  $total_count = $chado_property->get_total();
  $properties = $chado_property->get_props();

  $rows = tripal_curator_split_build_usage_table($properties);


  //pagination settings
  $per_page = 20;
  // Initialize the pager
  $current_page = pager_default_initialize(count($rows), $per_page);
  // Split your list into page sized chunks
  $chunks = array_chunk($rows, $per_page, TRUE);
  // Show the appropriate items from the list
  $output = theme('table', [
    'header' => ['Value'],
    'rows' => $chunks[$current_page],
  ]);


  $form['usage'] = ['#type' => 'fieldset', '#title' => 'Property values for this term (all tables)', '#collapsible' => TRUE];
  $form['usage']['table'] = [
    '#markup' => $output,
  ];

  if ($total_count === 0) {
    return ($form);
  }

  $regexp_description = "<p>To split a property, you must provide a regular expression (regexp) which matches your child term value.  Curator will find properties that match your regexp, create a new child property with this value, and update the parent property, removing the match.</p>
  
  <p>
  For example...  \"/, (.*)/\"
  </p>
  ";


  //TODO:  I NEED TO REFACTOR THIS!


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
      'callback' => "tripal_curator_split_destination_property_callback",
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


  //DONE WITH CVTERM FORM BOX.


  $form['regexp'] = [
    '#type' => 'textfield',
    '#title' => "Regular Expression",
    '#description' => $regexp_description,
  ];


  $form['test_regexp'] = [
    '#type' => 'button',
    '#value' => "Check Regular Expression",
    '#ajax' => [
      'callback' => 'tripal_curator_test_split_regexp_callback',
      'wrapper' => 'curator-split-wrapper',
      'effect' => 'fade',
    ],
  ];


  $form['output'] = [
    '#type' => 'fieldset',
    '#prefix' => '<div id="curator-split-wrapper">',
    '#suffix' => '</div>',
  ];

  // if regexp was provided, test how many terms and what the output will be.

  $regexp = NULL;
  if (isset($form_state['values']['regexp'])) {
    $regexp = $form_state['values']['regexp'];
  }

  if (!$regexp) {
    $form['output']['kill'] = ['#markup' => 'please provide a regular expression'];
    return $form;
  }


  $valid = tripal_curatori_is_regexp($regexp);

  if (!$valid) {
    $form['output']['kill'] = ['#markup' => 'please provide a valid regular expression.'];
    return $form;
  }

  $chado_property->match_records_against_regexp($regexp);

  $summary = $chado_property->get_split_summary();


  if (!$summary) {

    $form['output']['kill'] = ['#markup' => 'No records match the provided regular expression: <strong>' . $regexp . '</strong>'];
    return $form;
  }

  $header = ['Resulting Parent Value', 'Resulting Child Value'];


  foreach ($summary as $table_name => $usage) {
    $rows = [];
    $i = 0;

    $count = count($usage);

    $form['output']['summary'][$table_name]['intro'] = ['#markup' => 'There are <strong>' . $count . '</strong> properties in the ' . $table_name . ' table that will be split.  The first 10 are displayed below.'];


    foreach ($usage as $row) {
      if ($i <= 10) {

        $rows[] = [$row['parent'], $row['child']];

        $i++;
      }
    }

    $table = [
      'header' => $header,
      'rows' => $rows,
    ];

    $markup_table = theme('table', $table);

    $form['output']['summary'][$table_name][] = ['#markup' => $markup_table];
  }

  //TODO: I don't like having it here but needs to be in this fieldset to be redrawn.
  $form['output']['Submit'] = ['#type' => 'submit', '#value' => 'Submit'];

  return $form;

}


function tripal_curator_split_prop_form_submit($form, &$form_state) {

  dpm($form_state);
  $values = $form_state['values'];
  $source_cvterm = $values['source_cvterm_id'];
  $regexp = $values['regexp'];

  $selected_child_cvterm = NULL;

  foreach ($values as $value_key => $value) {

    if (preg_match('/term-/', $value_key)) {

      if ($value = "on") {
        $selected_child_cvterm = str_replace('term-', '', $value_key);
      }
    }
  }

  global $user;

  $options = [$source_cvterm, $selected_child_cvterm, $regexp];

  tripal_add_job("Split property on term $source_cvterm", 'tripal_curator', 'tripal_curator_split_terms', $options, $user->uid, 10);
  //Submit job

}

function tripal_curator_get_term_box($form, &$form_state) {

  return $form;
}

function tripal_curator_test_split_regexp_callback($form, &$form_state) {
  return $form['output'];
}


function tripal_curator_split_destination_property_callback($form, &$form_state) {
  return $form['destination_type'];
}

function tripal_curatori_is_regexp($string) {
  return @preg_match($string, '') !== FALSE;
}

function tripal_curator_split_build_usage_table($props) {


  $usage = [];

  foreach ($props as $table => $properties) {
    foreach ($properties as $prop) {
      $value = $prop->value;
      $usage[] = [$value];
    }
  }
  return $usage;
}