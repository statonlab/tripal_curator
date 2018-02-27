<?php

namespace tripal_curator;


class Chado_property {


  /**
   * The array of all properties using a given cvterm.
   *
   * @var array
   */
  private $properties = [];


  private $total_count = 0;

  private $counts_by_table = [];


  private $type_id = NULL;

  /**
   * Chado_property constructor.
   *
   * Initialize with nothing instead
   * Then provide search
   *
   * @param $cvterm_id
   */

  public function __construct() {

  }


  /**
   * @param $type_id
   *
   * @return array
   */
  public function set_cvtermprop_search($type_id) {
    $tables = tripal_curator_get_property_tables();
    $query = NULL;
    $results = [];
    $results_count = [];
    $count_all = 0;

    foreach ($tables as $table) {
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, ['type_id']);
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

    $this->type_id = $type_id;

    return ($results);
  }


  public function set_cvtermprop_value_search() {

  }

  public function set_cvtermprop_cvalue_search() {

  }

  /**
   * @return null
   */
  public function get_props() {

    return ($this->properties);
  }

  /**
   * @param $new_cvterm_id
   * @param $table
   *
   * @return bool
   */
  public function remap_property_specific($new_cvterm_id, $table) {

    //First, verify the cvterm exists

    $cvterm = tripal_get_cvterm(array(
      'cvterm_id' => $new_cvterm_id,
    ));

    if (!$cvterm){

      return FALSE;
    }
    $t = tripal_curator_chadofy($table);
//
//    $query = db_select($t, $table);
//    $query->fields($table, ['type_id']);
//    $query->condition('type_id', $type_id);
//    $result = $query->execute()->fetchAll();


  }

  /**
   * Changes the type_id of all properties in the object.
   */
  public function remap_property_all($new_cvterm_id) {


    $cvterm = tripal_get_cvterm(array(
      'cvterm_id' => $new_cvterm_id,
    ));


    if (!$cvterm){
  tripal_set_message("Unable to remap properties, invalid CVterm " . $new_cvterm_id .  " supplied.", TRIPAL_ERROR);
      return FALSE;
    }

    $properties = $this->properties;

    foreach($properties as $proptable){
      $ids = array_keys($proptable);

    }

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
  public function get_table_count($table) {
    if (isset($this->counts_by_table[$table])) {
      return $this->counts_by_table[$table];
    }
  }


  public function get_type_id() {
    return $this->type_id;
  }

}