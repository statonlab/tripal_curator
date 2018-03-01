<?php

namespace tripal_curator;


class Chado_property {


  /**
   * The type_id of the property.  corresponds to a cvterm_id.
   *
   * @var null
   */
  private $type_id = NULL;

  /**
   * The array of all properties using a given cvterm.
   *
   * @var array
   */
  private $properties = [];

  /**
   * Count of how many instances of the property there are.
   * //TODO: Is it unique instances?  Or do multiple biomaterials wit hte hsame
   * prop count for each.
   * //and which do iwant anyway?
   *
   * @var int
   */
  private $total_count = 0;

  /**
   * A nested array where the key is the property table name and the value is
   * the count
   *
   * @var array
   */
  private $counts_by_table = [];


  /**
   * The property fields that will be queried against.
   * Setup functions should change this to account for ['cvalue_id')
   *
   * @var array
   */
  private $property_fields_to_include = [];


  /**
   * Initialize the class with a type_id.
   *
   * @param $type_id
   *
   * @return array
   */
  public function set_cvtermprop_search($type_id) {
    $this->type_id = $type_id;
    //TODO: should we instead include the cvalue_id, and remove it it ifit doesnt exist in the loop via db_field_exists()?

    $this->property_fields_to_include = [
      'type_id',
      'value',
      'rank',
    ]; //can't assume we have cvalue_id.

    $tables = tripal_curator_get_property_tables();
    return $this->setup_property_by_tables($tables);
  }


  /**
   * Builds the class to hold all properties with no cvalue.
   *
   */
  public function build_blank_cvalues() {
    $tables = tripal_curator_get_property_tables();

    $this->type_id = NULL;//Null the type_id, we're looking for null here.

    $cvalue_tables = [];

    //remove prop tables without a cvalue_id, and update the fields array
    foreach ($tables as $table) {
      db_field_exists(tripal_curator_chadofy($table), 'cvalue_id') ? $cvalue_tables[] = $table : NULL;
    }

    if (count($cvalue_tables) === 0) {
      tripal_set_message("Looking for prop tables with cvalue_id but none found", TRIPAL_WARNING);
      return (FALSE);
    }
    $this->property_fields_to_include = [
      'type_id',
      'value',
      'cvalue_id',
      'rank',
    ];

    $properties = $this->setup_property_by_cvalue($cvalue_tables);

    return TRUE;
  }


  /**
   * Becaus we aren't specifying a type_id (might have multiple types) it's easier to have a separate less DRY command for searching for cvalues.
   * @param $tables
   */

  private function setup_property_by_cvalue($tables) {
    $query = NULL;
    $results = [];
    $results_count = [];
    $count_all = 0;
    $type_id = $this->type_id;
    $fields = $this->property_fields_to_include;// array of existing fields

    foreach ($tables as $table) {

      $table_fields = $fields;
      array_push($table_fields, $table . '_id');
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, $table_fields);

      if (!$type_id){
        $query->isNull('cvalue_id');
      } else {
        $query->condition('cvalue_id', $type_id);
      }

      $result = $query->execute()->fetchAll();

      if ($result) {
        $results[$table] = $result;
        $results_count[$table] = count($result);
        $count_all += count($result);
      }
    }
    $this->properties = $results;
    $this->counts_by_table = $results_count;
    $this->total_count = $count_all;

    return ($results);

  }

    /**
   * Method for populating or re-populating the class.
   *
   * @param $tables
   *
   * @return array
   */
  private function setup_property_by_tables($tables) {
    $query = NULL;
    $results = [];
    $results_count = [];
    $count_all = 0;
    $type_id = $this->type_id;
    $fields = $this->property_fields_to_include;// array of existing fields


    foreach ($tables as $table) {


      $table_fields = $fields;
      array_push($table_fields, $table . '_id');
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, $table_fields);
      $query->condition('type_id', $type_id);
      $result = $query->execute()->fetchAll();

      if ($result) {
        $results[$table] = $result;
        $results_count[$table] = count($result);
        $count_all += count($result);
      }
    }
    $this->properties = $results;
    $this->counts_by_table = $results_count;
    $this->total_count = $count_all;


    return ($results);

  }


  /**
   * returns all properties
   *
   * @return null
   */
  public function get_props() {

    return ($this->properties);
  }


  /**
   * Given an array of table names, rebuild the class to only contain
   * properties from those tables.
   *
   * @param $tables
   *
   * @return array
   */
  public function specify_tables($tables) {

    return $this->setup_property_by_tables($tables);

  }

  /**
   * Changes the type_id of all properties in the object.
   */
  public function remap_property_all($new_cvterm_id) {

    $cvterm = tripal_get_cvterm([
      'cvterm_id' => $new_cvterm_id,
    ]);


    if (!$cvterm) {
      tripal_set_message("Unable to remap properties, invalid CVterm " . $new_cvterm_id . " supplied.", TRIPAL_ERROR);
      return FALSE;
    }

    $properties = $this->properties;


    foreach ($properties as $proptable => $properties) {
      $record_id_key = $proptable . '_id';

      $record_ids = [];
      foreach ($properties as $property) {
        $type_id = $property->type_id;//
        $record_id = $property->$record_id_key;
        $record_ids[] = $record_id;
      }

      $t = tripal_curator_chadofy($proptable);
      $query = db_update($t)
        ->fields(['type_id' => $new_cvterm_id])
        ->condition($record_id_key, $record_ids, 'IN');
      $result = $query->execute();
    }

    $this->set_cvtermprop_search($new_cvterm_id);

  }

  public function get_total() {
    return $this->total_count;
  }

  /**
   * Returns the number of properties in that specific table in the object.
   *
   * @param $table
   *
   * @return mixed
   */
  public function get_table_count($table = NULL) {

    if (!$table) {
      return $this->counts_by_table;
    }
    if (isset($this->counts_by_table[$table])) {
      return $this->counts_by_table[$table];
    }
  }


  public function get_type_id() {
    return $this->type_id;
  }

}