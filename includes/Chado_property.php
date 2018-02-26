<?php

namespace tripal_curator;


class Chado_property {

  /**
   * If specifying a table, will only look in that prop table.
   * If Null, Class covers *all** prop tables
   *
   * @var null
   */
  private $table = NULL;

  /**
   * The
   *
   * @var array
   */
  private $properties = [];


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

    //This is how i would build it with a single query.
    //might lose out too much information doing it this way though
    //    foreach ($tables as $table) {
    //      $t = tripal_curator_chadofy($table);
    //      if (!$query) {
    //        $query = db_select($t);
    //      }
    //      else {
    //        $query->full_join($t);
    //      }
    //      $query->fields($t, ['type_id']);
    //
    //    }
    //    $query->condition('type_id', $type_id);

    foreach ($tables as $table) {
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, ['type_id']);
      $query->condition('type_id', $type_id);
      $result = $query->execute()->fetchAll();
      $results[$table] = $result;
      }
$this->properties = $results;

$this->cvterm_by_table();

    return($results);
  }

  private function cvterm_by_table() {

    $props = $this->properties;
    $count_by_table = [];

    foreach($props as $table){

    }

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


  public function remap_property() {


  }


}