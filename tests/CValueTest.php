<?php

namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use \tripal_curator\CValue;

//some reading for organizing and annotating tests
//https://stackoverflow.com/questions/8313283/phpunit-best-practices-to-organize-tests
//https://jtreminio.com/2013/03/unit-testing-tutorial-introduction-to-phpunit/

class CValueTest extends TripalTestCase {

  use DBTransaction;

  public $cvalue = NULL;

  public $cvterm = NULL;

  public $fake_biomaterial_name = "Tripal Curator testing biomaterial";

  public $fake_biomat_id = NULL;

  public function setUp() {
    parent::setUp();

    $cvterm = chado_insert_cvterm(
      [
        'name' => 'Curator Test',
        'definition' => 'A test CVterm.  Should be deleted in test.',
        'cv_name' => 'tripal',
        'is_relationship' => 0,
        'db_name' => 'tripal',
      ]
    );

    $this->cvterm = $cvterm;

    $biomaterial = factory('chado.biomaterial')->create(['name' => 'Tripal Curator testing biomaterial']);

    //insert a fake biomaterial


    if (!chado_table_exists('biomaterialprop', 't')){
      print("warning: biomaterialprop table doesnt exist!");
      exit ;
    }

    var_dump($cvterm);
    
    $prop = db_select('chado.biomaterialprop', 't')
      ->condition('t.biomaterial_id', $biomaterial->biomaterial_id)
      ->condition('t.type_id', $cvterm->cvterm_id)
      ->condition('t.value', 'Curator Test')
      ->condition('cvalue_id', $cvterm->cvterm_id)
      ->fields('t', ['type_id'])
      ->execute()->fetchObject();

    if (!$prop) {

      $query = db_insert('chado.biomaterialprop')
        ->fields([
          'biomaterial_id' => $biomaterial->biomaterial_id,
          "type_id" => $cvterm->cvterm_id,
          "value" => "Curator Test",
          'cvalue_id' => $cvterm->cvterm_id,
        ]);
      $result = $query->execute();

    }


    $cvterm = tripal_insert_cvterm(
      [
        'name' => 'Curator Test TARGET',
        'definition' => 'The target test cvterm.  Should be deleted in test.',
        'cv_name' => 'tripal',
        'is_relationship' => 0,
        'db_name' => 'tripal',
      ]
    );

    $cvalue = new CValue;
    $this->cvalue = $cvalue;


    $query = db_select('chado.biomaterial', 't')
      ->fields('t')
      ->condition('name', 'Tripal Curator testing biomaterial', '=');
    $result = $query->execute()->fetchObject();

    if (!$result) {
      print "ERROR: Class set up failed to get biomaterial";
    }

    $this->fake_biomat_id = $result->biomaterial_id;

  }

  public function test_initialize_property() {
    $cval = $this->cvalue;
    $this->assertInstanceOf(Cvalue::class, $cval);
  }

  public function test_initialize_biomat() {

    $biomat_id = $this->fake_biomat_id;

    $this->assertNotNull($biomat_id, "biomaterial id was null");

    $record = ['table' => "biomaterial", 'id' => $biomat_id];

    $query = db_select('chado.biomaterialprop', 't')
      ->fields('t')
      ->condition('biomaterial_id', $biomat_id, '=');
    $result = $query->execute()->fetchObject();

    $this->assertNotNull($result, "biomaterial property was null");

  }


  public function test_defining_by_text() {

    $cval = $this->cvalue;
    $cval->set_value_text("Curator Test");

    $props = $cval->get_properties();

    $this->assertNotEmpty($props);

    foreach ($props as $table => $properties) {
      $this->assertNotNull($table);
      $this->assertNotEmpty($properties);
    }

  }

  public function test_defining_by_cvalue_id() {
    $cval = $this->cvalue;

    //put fake cvterm in class
    $cvterm = tripal_get_cvterm(
      [
        'name' => 'Curator Test',
        'cv_id' => [
          'name' => 'tripal',
        ],
      ]
    );

    $cval->set_cvalue_search($cvterm->cvterm_id);

    $props = $cval->get_properties();

    $this->assertNotEmpty($props);


  }

  /**
   * @group failing
   * Tests the api edit form
   */

  public function test_assign_cvalue_id() {
    $cval = $this->cvalue;
    $cval->set_value_text("Curator Test");


    $cvterm = tripal_get_cvterm(
      [
        'name' => 'Curator Test TARGET',
        'cv_id' => [
          'name' => 'tripal',
        ],
      ]
    );

    $cval->reassign_cvalue($cvterm->cvterm_id);

    $props = $cval->get_properties();

    $properties = array_pop($props);
    $test_prop = array_pop($properties);


    $this->assertObjectHasAttribute('cvalue_id', $test_prop);
    $this->assertNotNull($test_prop->cvalue_id);
    $this->assertEquals($cvterm->cvterm_id, $test_prop->cvalue_id, "new cvalue_id does not match assigned id");

    $cvterm = tripal_get_cvterm(
      [
        'name' => 'Curator Test',
        'cv_id' => [
          'name' => 'tripal',
        ],
      ]
    );

    $cval->reassign_cvalue($cvterm->cvterm_id);
    $cval->set_value_to_cvalue();
  }

  public function test_assign_value_to_cvalues_name() {
    $cval = $this->cvalue;
    $cval->set_value_text("Curator Test");


    $cvterm = tripal_get_cvterm(
      [
        'name' => 'Curator Test TARGET',
        'cv_id' => [
          'name' => 'tripal',
        ],
      ]
    );

    $cval->reassign_cvalue($cvterm->cvterm_id);
    $cval->set_value_to_cvalue();

    $props = $cval->get_properties();


    $properties = array_pop($props);
    $test_prop = array_pop($properties);

    $this->assertEquals("Curator Test TARGET", $test_prop->value);

    $cvterm = tripal_get_cvterm(
      [
        'name' => 'Curator Test',
        'cv_id' => [
          'name' => 'tripal',
        ],
      ]
    );

    $cval->reassign_cvalue($cvterm->cvterm_id);
    $cval->set_value_to_cvalue();
  }
}