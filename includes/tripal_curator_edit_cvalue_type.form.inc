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

  $cvalue_ids = [];

  $cval = new \tripal_curator\CValue();

  $instruction_markup = NULL;

  $form['source_value'] = [
    '#type' => $type,
    '#value' => $value,
  ];

  $valid_options = ["text", "cvalue_id"];

  if (!in_array($type, $valid_options)){

    $form['rejected'] = tripal_curator_cvalue_form_rejection_builder();

    return $form;
  }

  if ($type == "text") {
    //not necessary with one source value?
    //    $form['source_text'] = [
    //      '#type' => 'value',
    //      '#value' => $value,
    //    ];
    $cval->set_value_text($value);

    $instruction_markup = "<p><strong>Property Selection Type</strong>: Text</p>
                            <p>The properties listed below all share the same text-based value.</p>";
    $instruction_markup .= "
             <p><strong>Value</strong>: {$value}</p>
        ";
  }

  if ($type == "cvalue_id") {

    $value = (int) $value;

    //    $form['source_cvalue_id'] = [
    //      '#type' => 'value',
    //      '#value' => $value,
    //    ];

    if (!$value ){
      $form['rejected'] = tripal_curator_cvalue_form_rejection_builder();

      return $form;
    }

    $cvterm = tripal_get_cvterm([
      'cvterm_id' => $value,
    ]);

    if (!$cvterm){
      $form['rejected'] = tripal_curator_cvalue_form_rejection_builder();

      return $form;
    }

    $cval->set_cvalue_search($value);
    $properties = $cval->get_properties();
    $total_count = $cval->get_total_count();

    $cvalue_ids[$value] = $value;//We do this because below, we'll list the cvalues associated with these properties, without having to worry about if we are doing free-text or cvalue.


    $cvterm_name = $cvterm->name;
    $cvterm_definition = 'Undefined.';


    if (isset($cvterm->definition)) {
      $cvterm_definition = $cvterm->definition;
    }

    if ($total_count === 0 ){

      $form['rejected'] = ['#markup' =>
        "There are no properties utilizing this cvterm for the value.  " . 'Please return to <a href=' . "/admin/tripal/extension/tripal_curator/property_values" . '>The curator cvalue select form.</a>'];

      return $form;
    }


    $instruction_markup = "<p><strong>Property Selection Type</strong>: Cvalue</p>
                            The properties listed below all share the same CVterm value (Cvalue).";

    $instruction_markup .= "<p><strong>Term Name</strong>: {$cvterm_name}</p>";
    $instruction_markup .= "<p><strong>Term Definition</strong>: {cvterm_definition}</p>";

  }

  // TODO: get total count of property records
  $total_count = 0;

  $instruction_markup .= '<p><b>Total Property Records: </b>' . $total_count . '</p>';
  $instruction_markup .= '<p><b>Property Tables included: </b></p><ul>';


  $type_ids = [];

  // TODO: whatever this is
//  foreach ($properties as $table => $props) {
//    $instruction_markup .= '<li><b>' . $table . '</b>: ' . count($props) . ' </li>';
//
//    foreach ($props as $prop) {
//      $type_ids[$prop->type_id] = $prop->type_id;
//
//      if ($type == "text") {
//        //There might be multiple cvalues for free text, or none.
//
//        if (property_exists($prop, 'cvalue_id')) {
//          $cvalue_ids[$prop->cvalue_id] = $prop->cvalue_id;
//        }
//      }
//    }
//  }


  $instruction_markup .= '</ul>';
  // TODO: display property types
  $instruction_markup .= '<p><b>Property Types included: </b></p><ul>';

  foreach ($type_ids as $cvterm_id) {
    $cvterm = tripal_get_cvterm(['cvterm_id' => $cvterm_id]);
    $instruction_markup .= '<li>' . $cvterm->name . '</li>';
  }
  $instruction_markup .= '</ul>';


  if ($type != "cvalue") {


    $instruction_markup .= '<p><b>Value Properties (Cvalues) included: </b></p><ul>';

    foreach ($cvalue_ids as $cvalue) {
      $cvterm = tripal_get_cvterm(['cvterm_id' => $cvalue]);
      $instruction_markup .= '<li>' . $cvterm->name . '</li>';
    }
    $instruction_markup .= '</ul>';
  }

  $form['Value Information'] = [
    '#type' => 'fieldset',
  ];

  $form['Value Information']['input'] = [
    '#markup' => $instruction_markup,
  ];

  //  $form['Property Table'] = [
  //    '#type' => 'fieldset',
  //  ];

  // $table = tripal_curator_edit_cvalue_provide_property_table($properties);


  //TODO does it make sense to push in choices based on the $type?
  $action_choices = [
    '' => '-Please Select-',
    "set_cvalue" => "Set Property Value CVterm",
    'split' => "Split Properties",

  ];


  $default = '';

  if (isset($form_state['values']) && isset($form_state['values']['action'])) {
    $default = $form_state['values']['action'];

  }

  $form['action'] = [
    '#type' => 'select',
    '#title' => 'CVterm value Curation Type',
    '#description' => 'Tripal Curator allows you to change the CVterms associated with the targeted property values.  You can choose to annotate these values with a CVterm.  Alternatively, you can split the properties into multiple properties sharing the same type.',
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

  if (isset($form_state['input']) && isset($form_state['input']['action'])) {
    $action = $form_state['input']['action'];
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
      '#type' => 'checkbox',
      '#title' => "Overwrite Text Value with Property Name",
      '#description' => "You can choose to overwrite the text value of the property with the name of your chosen value cvterm.",
    ];

  }

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => 'submit',
  ];

  return $form;

}

/**
 *  Hook validate.
 *
 * @param $form
 * @param $form_state
 */
function tripal_curator_edit_cvalue_form_validate($form, &$form_state) {

  $values = $form_state['values'];

  // $type = $values['type'];

  $action = $values['action'];

  // $source_val = $values['source_value'];

  if (!$form['source_value']) {
    tripal_set_message("No source value selected!  Can't batch change NULL values!", TRIPAL_ERROR);
  }

  if ($action == "set_cvalue") {

//    $overwrite = $values['overwrite_value'];
    $selected_target_cvterm = NULL;

    foreach ($values as $value_key => $value) {

      if (preg_match('/term-/', $value_key)) {

        if ($value === "on") {
          $selected_target_cvterm = str_replace('term-', '', $value_key);
        }
      }
    }

    if (!$selected_target_cvterm) {
      form_set_error('destination_type', t("You must select a valid destination CVterm for this value. You must press Lookup Term and select a CVterm."));
    }

//    if (!$overwrite) {
//      form_set_error('overwrite_value', t("Please select an overwrite option."));
//    }
  }

  if ($action == "split") {
    form_set_error('action', t("Split is not yet functional.  Please follow us on github to get updates!"));

  }
}

/**
 * @param $form
 * @param $form_state
 */
function tripal_curator_edit_cvalue_form_submit($form, &$form_state) {

  $values = $form_state['values'];
  $action = $values['action'];
  $selected_target_cvterm = NULL;
  $source_val = $form['source_value']['#value'];

  $type = $values['type'];
  $overwrite = isset($values['overwrite_value']) ? $values['overwrite_value'] : NULL;

  //Build the property object
  $prop_obj = new \tripal_curator\CValue();

  if ($type === "text") {
    $prop_obj->set_value_text($source_val);
  }
//  if ($type === "cvalue") {
//    $prop_obj->set_cvalue_search($source_val);
//  }

  if ($action == "set_cvalue") {

    foreach ($values as $value_key => $value) {

      if (preg_match('/term-/', $value_key)) {

        if ($value === "on") {
          $selected_target_cvterm = str_replace('term-', '', $value_key);
        }
      }
    }

    $prop_obj->set_cvalue_search();
    $prop_obj->reassign_cvalue($selected_target_cvterm);

    if ($overwrite) {//overwrite the text value with the new cvterm name
      $prop_obj->set_value_to_cvalue();
    }
  }

  if ($action == "split") {



    //TODO:  not written yet.
  }
}

function tripal_curator_action_callback($form, &$form_state) {
  return $form['action_type_wrapper'];
}

function tripal_curator_edit_cvalue_provide_property_table($properties) {

  $count_total = 0;
  $rows = [];
  $headers = ["table", "property value", "property cvterm"];

  foreach ($properties as $table => $props) {
    $count = 0;
    foreach ($props as $prop) {

      $cvalue_id = $prop->cvalue_id;
      $cvterm = tripal_get_cvterm(['cvterm_id' => $cvalue_id]);

      $rows[] = [$table, $prop->value, $cvterm->name];

      $count_total++;
    }
  }

  if ($count_total > 100) {

  }

  $table = theme('table', ['rows' => $rows, 'header' => $headers]);

  return ($table);

}


function tripal_curator_cvalue_form_rejection_builder(){
  $rejection = [
    '#markup' => 'Something went wrong with your request!  Please return to <a href=' . "/admin/tripal/extension/tripal_curator/property_values" . '>The curator cvalue select form.</a>'
  ];
  return $rejection;
}