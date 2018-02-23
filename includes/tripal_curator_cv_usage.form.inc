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
    '#markup' => $markup
  ];

  $cv_usage= tripal_curator_get_cv_usage();

  dpm($cv_usage);



  $header = ['CV', 'Entity types', 'Unique terms used',  'Total entities'];
  $rows = [];

  foreach ($cv_usage as $cv) {
    $cv_info = $cv['cv_info'];

   unset($cv['cv_info']);

    $cv_name = $cv_info->name;
    $cv_id = $cv_info->cv_info;

  $types_list = array_keys($cv);
  $entity_count = 0;
  $term_count = 0;

    foreach($cv as $prop_table){

      $pt_count = count($prop_table);


    }



    $edit_link = '<a href = "edit/' . $cv_id . '"> edit </a>';
    $rows[] = [
      'CV' => $cv_name,
      'Entity types' => implode("\n", $types_list),
      'Unique terms used' => $term_count,
      'Total entities' => "not implemented yet",// $entity_count,
      'Edit' => $edit_link,
    ];
  }

  $table = [
    'header' => $header,
    'rows' => $rows,
    'attributes' => [],
    'sticky' => FALSE,
    'caption' => 'Caption',
    'colgroups' => [],
    'empty' => '',
  ];
  $output = theme_table($table);

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