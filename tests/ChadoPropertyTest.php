<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use \tripal_curator\Chado_property;

//some reading for organizing and annotating tests
//https://stackoverflow.com/questions/8313283/phpunit-best-practices-to-organize-tests
//https://jtreminio.com/2013/03/unit-testing-tutorial-introduction-to-phpunit/

class ChadoPropertyTest extends TripalTestCase {


  public $cvterm_test;

  public $cvterm_existing;

  public $property;


  protected function setUp() {

    $cvterm = tripal_insert_cvterm(
      [
        'name' => 'Curator Test',
        'definition' => 'A test CVterm.  Should be deleted in test.',
        'cv_name' => 'tripal',
        'is_relationship' => 0,
        'db_name' => 'tripal',
      ]
    );


    $this->cvterm_test = $cvterm;

    $property = new Chado_property();


    $query = [
      'name' => 'comment',
      'cv_id' => ['name' => 'cvterm_property_type'],
    ];

    $cvprop_term = tripal_get_cvterm($query);
    $this->cvterm_existing = $cvprop_term;
    $property->set_cvtermprop_search($cvprop_term->cvterm_id);
    $this->property = $property;


    //create a biomaterial that will be not have a cvalue

    //insert a fake biomaterial
    $biomaterial_id = tripal_biomaterial_create_biomaterial("Tripal Curator blank cvalue", NULL, NULL, NULL, NULL, NULL);

    $query = db_insert('chado.biomaterialprop')
      ->fields([
        'biomaterial_id' => $biomaterial_id,
        "type_id" => $cvterm->cvterm_id,
        "value" => "No cvalue!",
        'cvalue_id' => NULL,
      ]);
    $result = $query->execute();
  }


  public function test_initialize_property() {
    $property = $this->property;
    $this->assertInstanceOf(Chado_property::class, $property);

  }

  public function test_set_cvtermprop_search_finds_properties() {
    $property = $this->property;
    $this->assertNotEmpty($property->get_props());
  }


  public function test_chadoprop_count_all() {
    $count = $this->property->get_total();
    $this->assertNotNull($count);

  }


  public function test_chadoprop_count_specific() {

    $count = $this->property->get_table_count("cvtermprop");
    $this->assertNotNull($count);

    $count = $this->property->get_table_count("analysisprop");
    $this->assertNull($count);

    $count = $this->property->get_table_count("seabass");
    $this->assertNull($count);

  }

  public function test_specify_tables() {

    $property = $this->property;

    $property->specify_tables(['cvtermprop']);
    $count = $this->property->get_table_count("cvtermprop");
    $this->assertNotEmpty($count);

    $this->property->set_cvtermprop_search($this->cvterm_term_existing);//specify all tables again...

  }

  public function testremap_property_all() {
    $this->property->remap_property_all($this->cvterm_test->cvterm_id);

    $this->assertEquals($this->cvterm_test->cvterm_id, $this->property->get_type_id());
    $this->assertNotEmpty($this->property->get_props());

  }


  public function test_build_blank_cvalues_finds_properties() {
    $temp_property = new Chado_property();

    $result = $temp_property->build_blank_cvalues();

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


  protected function tearDown() {

    $values = ['name' => 'Tripal Curator blank cvalue'];
    chado_delete_record('biomaterial', $values);

    $cvterm_existing = $this->cvterm_existing;
    $cvterm_test = $this->cvterm_test;


    //do another sweep for properties that we acciddentally changed to the test prop.
    $clean_property = new Chado_property();
    $clean_property->set_cvtermprop_search($cvterm_test->cvterm_id);
    $clean_property->remap_property_all($cvterm_existing->cvterm_id);


    $values = ['cvterm_id' => $cvterm_test->cvterm_id];
    chado_delete_record('cvterm', $values);

  }


}
