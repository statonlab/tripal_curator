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

  /**
   * Fills the property table of this cvalue with the properties of a given
   * property value.
   *
   * @param $text
   *  The string value of the property for which to search.
   *
   * @return NULL
   */
  public function set_value_text($text) {
    // Get all prop tables containing a cvalue_id column
    $tables = tripal_curator_get_property_tables_with_cvalues();

    $count_all = 0;
    $properties = [];

    foreach ($tables as $table) {
      // t = chado.table
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
      if ($results) {
        $properties[$table] = $results;
        $count_all += count($results);
      }
    }
    $this->properties_by_table = $properties;
    $this->total_count = $count_all;

    return NULL;
  }

  /**
   * Fills the property table of this cvalue with the properties of a given
   * property cvalue_id.
   *
   * @param $cvalue_id
   *  The integer cvalue_id of the property for which to search.
   *
   * @return NULL
   */
  public function set_cvalue_search($cvalue_id){
    // Get all prop tables containing a cvalue_id column
    $tables = tripal_curator_get_property_tables_with_cvalues();

    $count_all = 0;
    $properties = [];

    foreach ($tables as $table) {
      // t = chado.table
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, [
        $table . '_id',
        'type_id',
        'rank',
        'value',
        'cvalue_id',
      ]);
      $query->condition('cvalue_id', $cvalue_id);
      $results = $query->execute()->fetchAll();
      if ($results) {
        $properties[$table] = $results;
        $count_all += count($results);
      }
    }
    $this->properties_by_table = $properties;
    $this->total_count = $count_all;

    return NULL;
  }

  /**
   * Sets the Value to the cvalue's value (hah)
   *
   * @return string
   */
  public function set_value_to_cvalue() {
    $value = NULL;
    $cvterm = $this->_get_cvterm_for_cvalue();

    $cvterm_id = $cvterm->cvterm_id;

    if (!$cvterm_id){
      throw new \Exception("No Cvterm_id to set the property value to!");
    }

    if (isset($cvterm->name)) {
      $value = $cvterm->name;
    }

    if (!$this->properties_by_table) {
      throw new \Exception("Attempting to set the property value of NULL properties.");
      return FALSE;
    }

    foreach ($this->properties_by_table as $table => $properties) {

      $query = db_update(tripal_curator_chadofy($table));
      $query->fields([
        'value' => $value,
      ]);
      $query->condition('cvalue_id', $cvterm->cvterm_id, '=');

      $results = $query->execute();

      if (!$results) {
        tripal_set_message("Unable to update values for properties in" . $table . "\n ", ERROR);
        return FALSE;
      }
    }

    $this->update_properties();

    return $value;
  }

  /**
   * @param $cvalue_id
   *
   * @return array|bool
   */
  public function reassign_cvalue($cvalue_id) {

    $this->cvalue_id = $cvalue_id;

    $this->_get_cvterm_for_cvalue();

    if (!$this->properties_by_table) {
      tripal_set_message("Attempting to set the property value of NULL properties.", ERROR);
      return FALSE;
    }

    foreach ($this->properties_by_table as $table => $properties) {

      $record_ids = [];
      $record_id_handle = $table . "_id";

      foreach ($properties as $property) {
        $record_ids[] = $property->$record_id_handle;
      }
      $query = db_update(tripal_curator_chadofy($table));
      $query->fields([
        'cvalue_id' => $cvalue_id,
      ]);
      $query->condition($record_id_handle, $record_ids, 'IN');
      $results = $query->execute();

      if (!$results) {
        tripal_set_message("Unable to update values for properties in" . $table . "\n ", ERROR);
        return FALSE;
      }
    }
    //Now update the properties in this object

    $this->update_properties();

    return $this->properties_by_table;
  }

  /**
   * Returns the properties held by this cvalue object
   *
   * @return array
   */
  public function get_properties() {
    return $this->properties_by_table;
  }

  /**
   * Refresh all properties held by the object.
   *
   * @return array
   */

  public function update_properties() {

    $props_by_table = $this->properties_by_table;

    $new_props = [];
    $count_all = 0;

    foreach ($props_by_table as $table => $properties) {
      $record_ids = [];
      $record_id_handle = $table . "_id";

      foreach ($properties as $property) {
        $record_ids[] = $property->$record_id_handle;
      }
      // t = chado.table
      $t = tripal_curator_chadofy($table);
      $query = db_select($t, $table);
      $query->fields($table, [
        $record_id_handle,
        'type_id',
        'rank',
        'value',
        'cvalue_id',
      ]);
      $query->condition($record_id_handle, $record_ids, 'IN');
      $results = $query->execute()->fetchAll();

      if ($results) {
        $count_all += count($results);
        $new_props[$table] = $results;
      }
    }

    $this->properties_by_table = $new_props;
    $this->total_count = $count_all;

    return $this->properties_by_table;
  }

  /**
   * Gets cvterm of cvalue.  Handles the error.
   *
   * @param $cvalue_id
   *
   * @return array|bool|mixed
   */

  private function _get_cvterm_for_cvalue() {
    $cvalue_id = $this->cvalue_id;
    $cvterm = tripal_get_cvterm(
      [
        'cvterm_id' => $cvalue_id,
      ]);

    if (!$cvterm) {
      print("warning\n No cvterm for cvalue\n");
      tripal_set_message("Attempt to cvalue to undefined cvterm ID.", TRIPAL_ERROR);
      return FALSE;
    }
    return $cvterm;
  }

  public function get_total_count() {
    return $this->total_count;
  }
}