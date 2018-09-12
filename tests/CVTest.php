<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use \tripal_curator\includes\CV;
use StatonLab\TripalTestSuite\DBTransaction;


class CVTest extends TripalTestCase {

  use DBTransaction;


  private function create_CV_and_props(){
    $cv = factory('chado.cv')->create();

    $cvterms = factory('chado.cvterm', 5)
      ->create(['cv_id' => $cv->cv_id ]);

    $biomaterial = factory('chado.biomaterial')
      ->create();

    $props = [];

    foreach ($cvterms as $cvterm) {
      $prop = factory('chado.biomaterialprop')
        ->create([
          'type_id' => $cvterm->cvterm_id,
          'biomaterial_id' => $biomaterial->biomaterial_id]);
    $props[] = $prop;
    }

    return [
      'cv' => $cv,
      'props' => $props
    ];

  }


  public function test_CV_inits_and_gets_terms() {
    $vals = $this->create_CV_and_props();
    $cv_id = $vals['cv']->cv_id;
    $CV = new CV;
    $CV->set_id($cv_id);
    $terms = $CV->get_terms();
    $this->assertNotNull($terms);

    $this->assertNotEmpty($terms);

    foreach ($terms as $term) {
      $this->assertObjectHasAttribute("type_id", $term, "There was no type key for the term object.");
      $this->assertObjectHasAttribute("name", $term, "There was no term name key for the term object.");
      $this->assertObjectHasAttribute("value", $term, "there was no value key for the term object.");
    }
  }

  /**
   *
   */
  public function test_CV_returns_prop_tables_on_init() {
    $vals = $this->create_CV_and_props();
    $cv_id = $vals['cv']->cv_id;
    $CV = new CV;
    $CV->set_id($cv_id);

    $tables = $CV->get_prop_tables();

    $this->assertNotEmpty($tables, "The get_prop_tables() method failed to return property tables containing the CV (cvterm_property_type).");

  }

  public function test_CV_returns_specific_terms() {

    $vals = $this->create_CV_and_props();
    $cv_id = $vals['cv']->cv_id;
    $CV = new CV;
    $CV->set_id($cv_id);

    $tables = $CV->get_prop_tables();

    $terms = $CV->get_terms_specific([$tables[0]]);

    $this->assertNotNull($terms);
    $this->assertNotEmpty($terms);

  }

}