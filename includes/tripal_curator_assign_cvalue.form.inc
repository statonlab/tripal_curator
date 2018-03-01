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


  //Step two: Determine Cvalue

  //Step three: Determine if splitting

  //Step four: Split as necessary

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
      $form = ['#markup' => 'Please enter a Text value to search for.'];
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
      $value = l($value, '1');

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