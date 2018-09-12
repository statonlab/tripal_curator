<?php

namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use \tripal_curator\Chado_property;

class ChadoPropertyTest extends TripalTestCase {

  use DBTransaction;

  protected function setUp() {
    parent::setUp();
//
//    $cvterm = tripal_insert_cvterm(
//      [
//        'name' => 'Curator Test',
//        'definition' => 'A test CVterm.  Should be deleted in test.',
//        'cv_name' => 'tripal',
//        'is_relationship' => 0,
//        'db_name' => 'tripal',
//      ]
//    );
//
//    $this->cvterm_test = $cvterm;
//
//    $property = new Chado_property();
//
//    $query = [
//      'name' => 'comment',
//      'cv_id' => ['name' => 'cvterm_property_type'],
//    ];
//
//    $cvprop_term = tripal_get_cvterm($query);
//
//    $this->cvterm_existing = $cvprop_term;
//    $property->set_cvtermprop_search($cvprop_term->cvterm_id);
//    $this->property = $property;
//
//    //create a biomaterial that will be not have a cvalue
//
//    $biomaterial = factory('chado.biomaterial')->create();
//
//    $query = db_insert('chado.biomaterialprop')
//      ->fields([
//        'biomaterial_id' => $biomaterial->biomaterial_id,
//        "type_id" => $cvterm->cvterm_id,
//        "value" => "No cvalue!",
//        'cvalue_id' => NULL,
//      ]);
//    $result = $query->execute();
  }


  public function test_initialize_property() {
    $property = new Chado_property();

    $this->assertInstanceOf(Chado_property::class, $property);

  }


  public function test_set_cvtermprop_search_finds_properties() {
    $args =  $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $cvterms = $args['cvterms'];
    $term = $cvterms[0];

    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($term->cvterm_id);
    $props = $property->get_props();

    $this->assertNotEmpty($props);
  }


  public function test_chadoprop_count_all() {
    $args =  $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $cvterms = $args['cvterms'];
    $term = $cvterms[0];

    $property = new Chado_property();

    $count = $property->get_total();
    $this->assertNotNull($count);

  }


  /**
   * Test that get_table_count works as intended.
   *
   */
  public function test_chadoprop_count_specific() {
    $args = $this->create_test_props();
    $cvterms = $args['cvterms'];
    $term = $cvterms[0];

    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($term->cvterm_id);

    $count = $property->get_table_count("biomaterialprop");
    $this->assertNotNull($count);

    $count = $property->get_table_count("analysisprop");
    $this->assertNull($count);

    $count = $property->get_table_count("seabass");
    $this->assertNull($count);

  }

  /**
   */
  public function test_specify_tables() {

    $args = $this->create_test_props();
    $cvterms = $args['cvterms'];
    $term = $cvterms[0];

    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($term->cvterm_id);


    $property->specify_tables(['biomaterialprop']);
    $count = $property->get_table_count("biomaterialprop");
    $this->assertNotEmpty($count);

  }


  public function test_build_blank_cvalues_finds_properties() {

    $this->create_test_props();
    $temp_property = new Chado_property();

    $result = $temp_property->build_blank_cvalues();

    $this->create_test_props();

    if (!$result) {
      print("\n\nNo prop tables with cvalue_id in test environment, skipping test_build_blank_cvalues_finds_properties\n");
      return;
      //TODO: Instead, we could check if a table has the cvalue_id column, and insert a new test property with cvalue_id = null
    }

    $target_prop_tables = $temp_property->get_props();
    $this->assertNotEmpty($target_prop_tables, "Build blank cvalues did not find any properties with cvalue_ids but no cvalues.");

    $target_props = array_pop($target_prop_tables);
    $this->assertNotEmpty($target_props, "Build blank cvalues returned a table without no properties.");

    $prop = array_pop($target_props);

    $this->assertObjectHasAttribute("cvalue_id", $prop, "Property retrieved from build_blank_cvalues lacks cvalue_id key.");
    $this->assertNull($prop->cvalue_id, "Property retrieved from build_blank_cvalues has non-null cvalue_id");

  }


  private function create_test_props(){

    $cv = factory('chado.cv')->create();

    $cvterms = factory('chado.cvterm', 5)
      ->create(['cv_id' => $cv->cv_id ]);

    $biomaterial = factory('chado.biomaterial')
      ->create();

    $props = [];

    $values = ['a', 'b', 'c', 'four hours, 100 degrees', 'six days, 10 degrees'];
    $i = 0;

    foreach ($cvterms as $cvterm) {

      $value = $values[$i];
      $prop = factory('chado.biomaterialprop')
        ->create([
          'type_id' => $cvterm->cvterm_id,
          'biomaterial_id' => $biomaterial->biomaterial_id,
          'value' => $value]);
      $props[] = $prop;
      $i++;
    }

    return [
      'cv' => $cv,
      'cvterms' => $cvterms,
      'props' => $props
    ];

  }

  /**
   */
  public function testSplitter(){

   $args =  $this->create_test_props();
   $cv = $args['cv'];
   $properties = $args['props'];
   $cvterms = $args['cvterms'];

   $term = $cvterms[0];

    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($term->cvterm_id);

    $props = $property->get_props();

  }


}
