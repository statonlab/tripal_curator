<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use \tripal_curator\includes\CV;

class CVTest extends TripalTestCase {

  //Fake CVterm for testing
  public $CV;

  public function setUp() {

    $CV = new CV;

//We use this cv for testing because it's included in the chado install, linked to properties.
    $cv_record = tripal_get_cv(['name' => 'cvterm_property_type']);
    $CV->set_id($cv_record->cv_id);
    $this->CV = $CV;

  }

  public function test_CV_inits_and_gets_terms() {
    $CV = $this->CV;
    $terms = $CV->get_terms();
    $this->assertNotNull($terms);

    $this->assertNotEmpty($terms);

    foreach ($terms as $term) {
      $this->assertObjectHasAttribute("type_id", $term, "There was no type key for the term object.");
      $this->assertObjectHasAttribute("name", $term, "There was no term name key for the term object.");
      $this->assertObjectHasAttribute("value", $term, "there was no value key for the term object.");
    }
  }
  public function test_CV_returns_prop_tables_on_init() {
    $CV = $this->CV;
    $tables =$CV->get_prop_tables();

    $this->assertNotEmpty($tables, "The get_prop_tables() method failed to return property tables containing the CV (cvterm_property_type).");

  }

  public function test_CV_returns_specific_terms() {

    $CV = $this->CV;

    $tables =$CV->get_prop_tables();

    $terms = $CV->get_terms_specific([$tables[0]]);

    $this->assertNotNull($terms);
    $this->assertNotEmpty($terms);

  }

}