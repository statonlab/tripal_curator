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


/**
 * Stolen from the tripal_biomaterial API.  Reproduced here since we only use it in the tests and dont want to create a module dependency for this one method.
 * @param $biomaterial
 * @param $analysis_id
 * @param $organism_id
 * @param $biosourceprovider_id
 * @param $dbxref_id
 * @param $biomaterial_description
 *
 * @return mixed
 * @throws \Exception
 */
function tripal_curator_create_biomaterial(
  $biomaterial,
  $analysis_id,
  $organism_id,
  $biosourceprovider_id,
  $dbxref_id,
  $biomaterial_description
) {
  $biomaterial_result = chado_query('select biomaterial_id, taxon_id, description from {biomaterial} where name = :name limit 1', [':name' => $biomaterial])->fetchObject();

  // if sample_name already exists in chado db, print warning message
  if ($biomaterial_result->taxon_id) {
    print("WARNING: Biomaterial with sample_name of '$biomaterial' alreay exists in the database. Overwriting database entries for $biomaterial...\n");
  }

  if ($biomaterial_result->taxon_id and $biomaterial_result->taxon_id != $organism_id) {
    $sql = "SELECT common_name, genus, species FROM {organism} WHERE organism_id = :organism_id";
    $analysis_org = chado_query($sql, [":organism_id" => $organism_id])->fetchObject();
    $biomaterial_org = chado_query($sql, [":organism_id" => $biomaterial_result->taxon_id])->fetchObject();
    print "ERROR: Mismatch between the organism selected in the loader " . "and the organism of a biomaterial in the Chado database.\n" . "  Organism selected by loader: " . $analysis_org->genus . " " . $analysis_org->species . " (" . $analysis_org->common_name . ")\n" . "  Biomaterial organism: " . $biomaterial_org->genus . " " . $biomaterial_org->species . " (" . $biomaterial_org->common_name . ")\n" . "  On biomaterial: " . $biomaterial . "\n" . "Please ensure that the organism selected by the loader " . "and the organism for each biomaterial in the file to be loaded " . "are the same. If these organisms should not be the same, " . "delete the offending biomaterial in Chado or change the name of " . "the biomaterial to another unique name.\n";
    exit;
  }

  $biomaterial_id = $biomaterial_result->biomaterial_id;

  if ($analysis_id) {
    $analysis_name = chado_select_record('analysis', ['name'], ['analysis_id' => $analysis_id]);
  }

  if ($biomaterial_description == '' and $biomaterial_result->description != '') {
    $biomaterial_description = '';
  }

  elseif ($biomaterial_description == '' && $analysis_id) {
    $biomaterial_description = 'This biomaterial: ' . $biomaterial . ', was created for the analysis: ' . $analysis_name[0]->name;
  }

  $values = [
    'name' => $biomaterial,

    'taxon_id' => $organism_id,
  ];

  if (!$biomaterial_id) {
    $biomaterial = chado_insert_record('biomaterial', $values);
    if ($biomaterial == FALSE) {
      print "ERROR: Unable to write to biomaterial table\n";
      exit;
    }
  }

  if ($biomaterial_id) {
    print("Updating record for biomaterial id = $biomaterial_id\n");
    $biomaterial = chado_update_record('biomaterial', ['name' => $biomaterial], $values, ['return_record' => TRUE]);
    if ($biomaterial == FALSE) {
      print "ERROR: Unable to update biomaterial_table\n";
      exit;
    }
  }
  $biomaterial_id = $biomaterial['biomaterial_id'];

  return $biomaterial_id;
}
