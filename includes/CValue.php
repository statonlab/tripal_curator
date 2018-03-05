<?php

namespace tripal_curator;


/*
 * This class is similar to Chado_property, but is for **CVALUES** instead of **type_ids.
 *
 * Probably should  define a base class, and have this/Chado_property extend it.
 *
 */

class CValue {


  private $cvalue_id = NULL;

  private $value_text = NULL;

  private $type_ids = [];

  private $properties_by_table = [];

  private $total_count = 0;


  public function set_value_text($text) {

    $tables = tripal_curator_get_property_tables_with_cvalues();

    var_dump($tables);

    $count_all = 0;
    $properties = [];

    $types = [];

    foreach ($tables as $table) {
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, [
        $table . '_id',
        'type_id',
        'rank',
        'value',
        'cvalue_id',
      ]);
      $query->condition('value', $text);
      $results = $query->execute()->fetchAll();
      dpm($results);
      if ($results) {
        $properties[$table] = $results;
        $count_all += count($results);

        foreach ($results as $result) {

        }
      }
    }


    $this->properties_by_table = $properties;
    $this->total_count = $count_all;


    return NULL;
  }


  public function get_properties() {

    return $this->properties_by_table;

  }
}