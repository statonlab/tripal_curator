<?php

namespace tripal_curator;


use PHPUnit\Runner\Exception;

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
   * A summary of the proposed regexp split property.  Of the form
   * [ $table => [ $value => [
   *  "parent" => new parent value (child excised)
   *  "child" => new child value (match of Regexp)
   *  ] ] ]
   *
   * @var array
   */
  private $split_summary = [];


  /**
   * If splitting a property, this is the "destintation" term for the child
   *
   * @var string
   */
  private $child_term_id = NULL;

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
   * Becaus we aren't specifying a type_id (might have multiple types) it's
   * easier to have a separate less DRY command for searching for cvalues.
   *
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

      if (!$type_id) {
        $query->isNull('cvalue_id');
      }
      else {
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

      //also get root FK column ie thing_id from thingprop

      $base_id = str_replace("prop", "", $table);
      $base_id = $base_id . "_id";

      $table_fields[] = $base_id;

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

  /**
   *
   *
   * @param $regexp | A  regular expression string.  It should uniquely match
   *   the property one wants to split off.
   *
   * @return array | Returns an array of the properties that match the regexp.
   */
  public function match_records_against_regexp($regexp) {
    $matched_records = [];

    $match_summary = [];

    $tables = $this->properties;

    foreach ($tables as $table => $props) {
      $table_matches = [];

      foreach ($props as $prop) {

        $matches = [];
        $match = preg_match($regexp, $prop->value, $matches);

        if ($match) {

          unset($matches[0]);
          if (count($matches) > 1) {

            print("warning: Too many matches for proprerty: of type " . $prop->type_id . "with value " . $prop->value . "\n");
            //TODO: proper exception handling.
            continue;
          }
          $child_match = $matches[1];

          $table_matches[] = $prop;

          $new_parent = preg_replace($regexp, '', $prop->value);
          $id = $prop->type_id;

          $match_summary[$table][$prop->value] = [
            'parent' => $new_parent,
            'child' => $child_match,
          ];

        }
      }
      if (!empty($table_matches)) {
        $matched_records[$table] = $table_matches;
      }
    }
    $this->split_summary = $match_summary;
    $this->properties = $matched_records;

    return $matched_records;
  }


  public function set_child_term($cvterm_id) {

    $this->child_term_id = $cvterm_id;

  }

  public function split_term_by_value_regexp() {

    $properties = $this->properties;
    $child_term = $this->child_term_id;
    $split_plan = $this->split_summary;

    foreach ($properties as $table => $props) {

      foreach ($props as $prop) {

        $lookup = $split_plan[$table][$prop->value];
        $new_child = $lookup['child'];
        $new_parent = $lookup['parent'];
        $parent_type = $prop->type_id;

        $key = $table . "_id";

        $base_table = str_replace("prop", "", $table);


        $record_id = $prop->$key;

        $record = ['table' => $base_table, 'id' => $record_id];

        $chado_property = [
          'type_id' => $child_term,
          'value' => $new_child,
        ];

        //not sure if we should update if present or not.  Don't want to accidentally overwrite existing properties.  Maybe it should check if the term is already set and, if so, set the rank to two?

        $options = [];

        chado_insert_property($record, $chado_property, $options);


        //$record stays the same

        $chado_property = [
          'type_id' => $parent_type,
          'value' => $new_parent,
        ];

        $options = ['update_if_present' => TRUE];

        chado_insert_property($record, $chado_property, $options);
      }
    }
  }


  public function get_split_summary() {
    return $this->split_summary;
  }

  private function create_new_property($prop, $child, $child_cvterm_id) {

  }

  private function update_property($prop, $new_parent) {

  }
}