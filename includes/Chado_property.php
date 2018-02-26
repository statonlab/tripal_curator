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


  private $total_count = 0;

  private $counts_by_table = [];

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

  private function cvterm_by_table() {

    $props = $this->properties;

    foreach ($props as $table) {
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