<?php

/**
 * Get all chado tables that are prop tables
 *
 * @return array
 */
function tripal_curator_get_property_tables() {

  $tables = [];
  $sql_table_list = "SELECT table_name FROM information_schema.tables WHERE table_schema = 'chado' ORDER BY table_name;";
  $result_table = db_query($sql_table_list);
  $input_table = $result_table->fetchAll();
  foreach ($input_table as $value) {

    //confirm that the table contains, and ends with, "prop"
    if (substr_compare($value->table_name, "prop", strlen($value->table_name) - strlen("prop"), strlen("prop")) === 0 && $value->table_name != "chadoprop") {

      $tables[] = $value->table_name;
    }
  }
  return $tables;
}

/**
 * Get all Chado property tables that have the cvalue_id column.
 * Because this isnt in the official Chado release as of this writing, not all
 * property tables will have the column.
 *
 * @return array
 */
function tripal_curator_get_property_tables_with_cvalues() {
  $tables = tripal_curator_get_property_tables();

  $output_tables = [];

  foreach ($tables as $table) {

    if (db_field_exists(tripal_curator_chadofy($table), 'cvalue_id')) {
      $output_tables[] = $table;
    }
  }

  return $output_tables;

}


/**
 * Retrieve all properties for a given prop table.
 *
 * @param $table
 *
 * @return null
 */
function tripal_curator_fetch_props_for_prop_table($table) {
  //check that this is a prop table

  if (substr_compare($table, "prop", strlen($table) - strlen("prop"), strlen("prop")) != 0) {
    tripal_set_message("Table " . $table . " is not a prop table, cannot fetch props.", TRIPAL_WARNING);
    return NULL;
  }


  $base_table = tripal_curator_str_lreplace('prop', '', $table);

  $query = db_select(tripal_curator_chadofy($table), 'PT'); //prop table
  $query->fields('PT', [$base_table . '_id', 'type_id', 'value']);
  $query->innerJoin(tripal_curator_chadofy('cvterm'), 'CVT', 'CVT.cvterm_id = PT.type_id');
  $query->fields('CVT', ['cv_id', 'name', 'definition', 'dbxref_id']);
  $query->innerJoin(tripal_curator_chadofy('dbxref'), 'DBX', 'CVT.dbxref_id = DBX.dbxref_id');
  $query->fields('DBX', ['db_id', 'accession']);

  $results = $query->execute()->fetchAll();

  return ($results);
}


/**
 * Replaces the last occurence of a substring in a string
 *
 * @param $search - The substring to match.
 * @param $replace - What you will replace the match with.
 * @param $subject - The string to search within.
 *
 * @return mixed
 */
function tripal_curator_str_lreplace($search, $replace, $subject) {
  $pos = strrpos($subject, $search);

  if ($pos !== FALSE) {
    $subject = substr_replace($subject, $replace, $pos, strlen($search));
  }
  return $subject;
}


/**
 * Affixes the chado db prefix (chado.) to a string
 *
 * @param $string
 *
 * @return string
 */
function tripal_curator_chadofy($string) {

  return ('chado.' . $string);
}


function tripal_curator_get_props_for_cv($cv_id) {

$out = [];
  $prop_tables = tripal_curator_get_property_tables($desired_cv = NULL);

  foreach ($prop_tables as $prop_table) {
    $query = db_select(tripal_curator_chadofy($prop_table), 't');
    $query->join('chado.cvterm', 'cvt', 'cvt.cvterm_id = t.type_id');
    $query->join('chado.cv', 'cv', 'cv.cv_id = cvt.cv_id');
    $query->fields('t', ['value', 'type_id']);
    $query->fields('cvt', ['name']);
    $query->distinct();
    $query->condition('cv.cv_id', $cv_id);
    $results = $query->execute()->fetchAll();
    if ($results){
      $out[$prop_table] = $results;
    }

  }
  return $out;
}
/**
 * get CV usage information at the CV level, for all prop tables.
 * @return array
 */
function tripal_curator_get_prop_by_cv_count() {

  $cv_usage = [];

  $prop_tables = tripal_curator_get_property_tables($desired_cv = NULL);

    foreach ($prop_tables as $prop_table) {

    $query = db_select(tripal_curator_chadofy($prop_table), 't');
    $query->join('chado.cvterm', 'cvt', 'cvt.cvterm_id = t.type_id');
    $query->join('chado.cv','cv', 'cv.cv_id = cvt.cv_id');
    $query->fields('cv', ['cv_id', 'name']);
    $query->distinct();
    $cvs = $query->execute()->fetchAll();

    foreach ($cvs as $cv){
      $query = db_select(tripal_curator_chadofy($prop_table), 't');
      $query->join('chado.cvterm', 'cvt', 'cvt.cvterm_id = t.type_id');
      $query->groupBy('cvt.cvterm_id');
      $query->condition('cvt.cv_id', $cv->cv_id);
      $count = $query->countQuery()->execute()->fetchField();
      $cv->count = $count;
      $cv_usage[$prop_table][] = $cv;
    }
    }

  if ($desired_cv) {
      $output_desired = [];
      dpm($desired_cv);
      foreach ($cv_usage as $prop_table => $cvs){

      }

    return $cv_usage[$desired_cv];
  }

  return $cv_usage;
}

/**
 * Creates a fieldset for the split property feature, giving the user the
 * interface needed to create a child property.
 *
 * @param $parent
 *  The child property under which to create the new property. Only needs to be
 *  specified if you're splitting a child property.
 *
 */
function tripal_curator_split_property_add_child(&$form, $form_state, $parent = NULL) {
  $count = 1;

  // Find the first occurrence where a split does not exist
  while (array_key_exists('split_' . $count, $form['action_type_wrapper'])) {
    ++$count;
  }

  $split_key = 'split_' . $count;

  // Create the fieldset
  $form['action_type_wrapper'][$split_key] = [
    '#type' => 'fieldset',
    '#title' => t('Child ' . $count),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  // Create the value textfield
  $form['action_type_wrapper'][$split_key]['value_text'] = [
    '#title' => 'Value Text',
    '#description' => 'The value free text that describes this property.',
    '#type' => 'textfield',
    '#required' => TRUE,
  ];
  // Create the property type textfield
  $form['action_type_wrapper'][$split_key]['type'] = [
    '#title' => 'Property Type',
    '#description' => 'The type of this property.',
    '#type' => 'textfield',
    '#required' => TRUE,
  ];
  // Create the cvterm browser
}

function tripal_curator_split_terms($parent_cvterm, $child_cvterm, $regexp){

  $chado_property = new \tripal_curator\Chado_property();
  $chado_property->set_cvtermprop_search($parent_cvterm);
  $chado_property->set_child_term($child_cvterm);
  $results = $chado_property->match_records_against_regexp($regexp);

  if (empty($results)){
    print("\n Error: No terms found when regexp was $regexp for parent term $parent_cvterm.");
    return;
  }
  $chado_property->split_term_by_value_regexp();

}