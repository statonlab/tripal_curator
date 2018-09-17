<?php


/**
 * @param $cvterm_id
 * @param $term_object
 *
 * @return array
 */
function provide_summary_box($cvterm_id, $term_object){

  $form = [];

  $type_id = $cvterm_id;


  $form['source_cvterm_id'] = [
    '#type' => 'value',
    '#value' => $type_id
  ];

  ### Display type_id info

  //I dont end up doing a whole lot with this class... maybe dont need
  $cvterm = $term_object;
  $cvterm->set_id($type_id);
  $cvterm_record = $cvterm->get_full();

  //instead, it would be

  // $cvterm = tripal_get_cvterm(array(
  //'cvterm_id' => $this->cvterm_id,
  //  ));

  $cvterm_name = $cvterm_record->name;
  $cvterm_definition = 'Undefined.';

  if (isset($cvterm_record->definition)){
    $cvterm_definition = $cvterm_record->definition;
  }


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

  if ($total_count === 0){

    $form['Source Type']['kill'] = [
      '#markup' => "No properties utilize this CVterm as their type!",
    ];
    return($form);

  }

  return $form;

}