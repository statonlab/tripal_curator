<?php

/**
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */

function tripal_curator_cv_usage_form($form, &$form_state) {

  $markup = "<h3>Tripal Curator CV usage</h3>
   <p>
This page is for managing your properties by controlled vocabulary.  Some modules previously used <strong>custom </strong> CVs that are not meaningful outside of the site.  Here, you can target properties using this CVs.
</p>
";

  $form['welcome'] = [
    '#markup' => $markup,
  ];

  $cv_usage = tripal_curator_get_cv_usage();

  $header = [
    'CV',
    'Entity types',
    'Unique terms used',
    'Total entities',
    "Edit usage",
  ];
  $rows = [];

  foreach ($cv_usage as $cv) {
    $cv_info = $cv['cv_info'];

    unset($cv['cv_info']);

    $cv_name = $cv_info->name;
    $cv_id = $cv_info->cv_id;

    $types_list = array_keys($cv);
    $entity_count = 0;
    $term_count = 0;

    foreach ($cv as $prop_table) {
      $term_count += count($prop_table);
    }


    $edit_link = '<a href = "CV_usage/' . $cv_id . '"> edit </a>';
    $rows[] = [
      'CV' => $cv_name,
      'Entity types' => implode(", ", $types_list),
      'Unique terms used' => $term_count,
      'Total entities' => "not implemented yet",// $entity_count,
      'Edit' => $edit_link,
    ];
  }

  //pagination settings
  $per_page = 20;
  // Initialize the pager
  $current_page = pager_default_initialize(count($rows), $per_page);
  // Split your list into page sized chunks
  $chunks = array_chunk($rows, $per_page, TRUE);
  // Show the appropriate items from the list
  $output = theme('table', [
    'header' => $header,
    'rows' => $chunks[$current_page],
  ]);
  // Show the pager
  $output .= theme('pager', ['quantity', count($rows)]);

  $form['Table'] = [
    '#type' => 'markup',
    '#markup' => $output,
  ];


  return $form;
}


function tripal_curator_cv_cvterms_form($form, &$form_state, $cv_id) {

  //get all cvterms for this cv

  $cvterms = new \tripal_curator\includes\CV();

  $cvterms->set_id($cv_id);

  $cv_name = $cvterms->get_cv_name();


  $tables = $cvterms->get_prop_tables();

  $tables_convert = [];
  foreach ($tables as $table) {
    $tables_convert[$table] = $table;
  }

  if (count($tables) == 0) {

    $message = "<p>The CV " . $cv_name . " is not utilized in any property tables.</p>";
    $form['message'] = [
      '#markup' => $message,
    ];

  }

  $form['check_proptables'] = [
    '#title' => 'Select Property Tables',
    '#type' => 'checkboxes',
    '#description' => 'Tripal curator will allow you to redefine the CVterms for properties in all tables or specific tables.  Check the property tables you would like to modify.',
    '#options' => $tables_convert,
    '#default_value' => array_keys($tables_convert),
    '#ajax' => [
      'callback' => 'tripal_curator_cv_filter_cvterms_callback',
      'wrapper' => 'curator_filtered_cvterms',
      'effect' => 'fade',
    ],
  ];


  $form['cvterms'] = [
    '#type' => 'fieldset',
    '#prefix' => '<div id="curator_filtered_cvterms">',
    '#suffix' => '</div>',
  ];

  $included = NULL;

  if (!isset($form_state['values'])) {
    $form_state['values']['check_proptables'] = $tables_convert;
  }

  if (isset($form_state['values']) && isset($form_state['values']['check_proptables'])) {
    $included = $form_state['values']['check_proptables'];

  }


  $rows = [];
  $header = ['CVterm'];


  $terms = $cvterms->get_terms_specific(array_keys($included));

  foreach ($terms as $term) {
    $name = NULL;
    $values = [];
    $instance_count = 0;

    foreach ($term as $term_instance) {

      $row = [];
      $type = $term_instance['type'];
      $name = $term_instance['term_name'];
      $value = $term_instance['value'];

      $instance_count++;
    }

    $name = l($name . ": " . $instance_count . " uses", 'admin/tripal/extension/tripal_curator/edit_type_id/' . $type);

    $row = [$name];
    $rows[] = $row;
  }
  dpm($rows);
  $table = ['rows' => $rows, 'header' => $header];

  dpm($table);


  $form['cvterms']['table'] = [
    '#markup' => theme('table', $table),
  ];


  return $form;
}


function tripal_curator_cv_filter_cvterms_callback($form, &$form_state) {

  return $form['cvterms'];

}