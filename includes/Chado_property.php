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
   *  Search in all prop tables for the given cvterm in the type_id column
   *
   * @param $type_id - The CVterm ID
   */
  public function set_cvtermprop_search($type_id) {
    $tables = tripal_curator_get_property_tables();
    $query = NULL;

    foreach ($tables as $table) {
      $t = tripal_curator_chadofy($table);
      if (!$query) {
        $query = db_select($t);
      }
      else {
        $query->full_join($t);
      }
      $query->fields($t, ['type_id']);

    }
    $query->condition('type_id', $type_id);

    $results = $query->execute()->fetchAll();

    $this->properties = $results;

  }

  public function set_cvtermprop_value_search() {

  }

  public function set_cvtermprop_cvalue_search() {

  }

  /**
   * @return null
   */
  public function get_props() {
    return ($this->table);
  }


  public function remap_property() {


  }


}