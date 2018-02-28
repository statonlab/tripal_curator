<?php


namespace tripal_curator\includes;


class CV {


  private $cv_id;

  private $cv_name;

  private $cv_definition;

  private $terms;

  private $prop_tables = [];

  private $cvterms_by_prop_table;


  public function set_id($id) {
    $this->cv_id = $id;

    //set all prop tables
    $usage = tripal_curator_get_cv_usage($id);

    if ($usage) {

      $cv_info = $usage["cv_info"];
      $this->cv_name = $cv_info->name;
      $this->cv_definition = $cv_info->definition;
      unset($usage["cv_info"]);


      $this->prop_tables = array_keys($usage);

      $this->cvterms_by_prop_table = $usage;

    }
  }


  /**Returns all terms stored in the object.
   * To return only terms for a specific prop table, use get terms specific instead, or, use set props first.
   *
   * @return array
   */
  public function get_terms() {

    $terms = [];

    foreach ($this->cvterms_by_prop_table as $prop_table => $props) {

      foreach ($props as $term) {
        $terms[$term["type"]] = $term;
      }
    }
    return $terms;
  }

  /**
   * Returns specific cvterms for a given array of prop tables.
   *
   * @param $tables  An array of prop table names.
   *
   * @return array
   *
   */

  public function get_terms_specific($tables) {

    $output = [];

    $usage = $this->cvterms_by_prop_table;
    foreach ($tables as $table) {

      $output[$table] = $usage[$table];
    }

    return $output;
  }


  /**
   * Modifies the class to only contain a subset of props
   *
   * @param $prop_tables
   *
   * @return array
   */
  public function set_props($prop_tables) {
    $this->prop_tables = $prop_tables;

    $term_list = [];

    foreach ($prop_tables as $prop_table) {


    }
    return $term_list;
  }

  /**
   * @return array
   */
  public function get_prop_tables() {
    return $this->prop_tables;
  }

  public function get_cv_name() {
    return $this->cv_name;
  }

}