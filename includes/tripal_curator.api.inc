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
 * Because this isnt in the official Chado release as of this writing, not all property tables will have the column.
 *
 * @return array
 */
function tripal_curator_get_property_tables_with_cvalues(){
$tables = tripal_curator_get_property_tables();

$output_tables = [];

foreach ($tables as $table){

  if (db_field_exists(tripal_curator_chadofy($table), 'cvalue_id')){
    $output_tables[] = $table;
  }
}

  return $output_tables;

}


function tripal_curator_fetch_props_for_prop_table($table) {
  //check that this is a prop table

  if (substr_compare($table, "prop", strlen($table) - strlen("prop"), strlen("prop")) != 0) {
    tripal_set_message("Table " . $table . " is not a prop table, cannot fetch props.", TRIPAL_WARNING);
    return NULL;
  }


  $base_table = tripal_curator_str_lreplace('prop', '', $table);

  $query = db_select(tripal_curator_chadofy($table), 'PT'); //prop table
  $query->fields('PT', [$base_table . '_id', 'type_id', 'value']);
  //TODO: conditionally add cvlaue_id here
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

/**
 * Goes through all properties in all chado property tables and indexes what
 * CVs are used where.

 * @param null $desired_cv  - If a cv_id is given, will return this cv's usage only.
 *
 * @return array|mixed
 */
function tripal_curator_get_cv_usage($desired_cv = null) {

  $cv_usage = [];

  $prop_tables = tripal_curator_get_property_tables();
  foreach ($prop_tables as $prop_table) {
    $props = tripal_curator_fetch_props_for_prop_table($prop_table);

    foreach ($props as $prop) {
      $cv = $prop->cv_id;
      $type = $prop->type_id;
      $term_name = $prop->name;
      $value_text = $prop->value;
      $db = $prop->db_id;

      $cv_array = [];
      if (isset($cv_usage[$cv])) {
        $cv_array = $cv_usage[$cv];
      }else {
        //populate basic info about this CV
          $cv_expanded = chado_generate_var('cv', ['cv_id' =>$cv]);
        $cv_array["cv_info"] = $cv_expanded;
      }

      //TODO:  this is wrong.  value needs to be an ARRAY of values (each useage case).
      //In addition I'd like to track the record_ids as well
      $cv_array[$prop_table][$type] = [
        'type' => $type,
        'term_name' => $term_name,
        'value' => $value_text,
      ];
      $cv_usage[$cv] = $cv_array;
    }
  }

  if ($desired_cv){
    return $cv_usage[$desired_cv];
  }


  return ($cv_usage);
}