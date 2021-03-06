<?php

namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use \tripal_curator\Chado_property;

class ChadoPropertyTest extends TripalTestCase {

  use DBTransaction;

  public function test_initialize_property() {
    $property = new Chado_property();

    $this->assertInstanceOf(Chado_property::class, $property);

  }


  public function test_set_cvtermprop_search_finds_properties() {
    $args = $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $term = $args['cvterm'];

    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($term->cvterm_id);
    $props = $property->get_props();

    $this->assertNotEmpty($props);
  }


  public function test_chadoprop_count_all() {
    $args = $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $term = $args['cvterm'];

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
    $term = $args['cvterm'];

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
    $term = $args['cvterm'];

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


  private function create_test_props() {

    $cv = factory('chado.cv')->create();

    $cvterm = factory('chado.cvterm')
      ->create(['cv_id' => $cv->cv_id]);

    $biomaterials = factory('chado.biomaterial', 5)
      ->create();

    $props = [];

    $values = [
      'a',
      'b',
      'c',
      'four hours, 100 degrees',
      'six days, 10 degrees',
    ];

    $i = 0;

    foreach ($biomaterials as $biomaterial) {
      $value = $values[$i];

      $prop = factory('chado.biomaterialprop')
        ->create([
          'type_id' => $cvterm->cvterm_id,
          'biomaterial_id' => $biomaterial->biomaterial_id,
          'value' => $value,
        ]);

      $props[] = $prop;

      $i++;
    }


    return [
      'cv' => $cv,
      'cvterm' => $cvterm,
      'props' => $props,
    ];

  }

  /**
   */
  public function testRegexpMatcher() {

    $args = $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $cvterm = $args['cvterm'];


    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($cvterm->cvterm_id);

    $props = $property->get_props();

    //Based on the create_test_props, we have two props with a comma for value.
    //'four hours, 100 degrees', 'six days, 10 degrees'
    //So we're hoping to return these two
    $qualifiers = $property->match_records_against_regexp('/, (.*)/');
    $this->assertNotEmpty($qualifiers);

    $this->assertArrayHasKey('biomaterialprop', $qualifiers);
    $bmats = $qualifiers['biomaterialprop'];

    $this->assertEquals(2, count($bmats));

    $summary = $property->get_split_summary();

    $this->assertArrayHasKey('biomaterialprop', $summary);
    $results = $summary['biomaterialprop'];
    $this->assertArrayHasKey('four hours, 100 degrees', $results);
    $to_test = $results['four hours, 100 degrees'];
    $this->assertArrayHasKey('child', $to_test);
    $this->assertArrayHasKey('parent', $to_test);
    $this->assertEquals('100 degrees', $to_test['child']);
    $this->assertEquals('four hours', $to_test['parent']);
  }

  /**
   * Ensure invalid Regexp at least runs
   *
   */
  public function testRegexpMatcher_invalid_regexp() {


    $args = $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $cvterm = $args['cvterm'];


    $property = new Chado_property();

    $tables = $property->set_cvtermprop_search($cvterm->cvterm_id);

    $props = $property->get_props();

    //Based on the create_test_props, we have two props with a comma for value.
    //'four hours, 100 degrees', 'six days, 10 degrees'
    //So we're hoping to return these two
    $qualifiers = $property->match_records_against_regexp('waffles');
    $this->assertEmpty($qualifiers);

  }


  /**
   */
  public function test_split_term_by_value_regexp() {

    $args = $this->create_test_props();
    $cv = $args['cv'];
    $properties = $args['props'];
    $cvterm = $args['cvterm'];

    $property = new Chado_property();
    $tables = $property->set_cvtermprop_search($cvterm->cvterm_id);
    $props = $property->get_props();

    //Based on the create_test_props, we have two props with a comma for value.
    //'four hours, 100 degrees', 'six days, 10 degrees'
    //So we're hoping to return these two
    $qualifiers = $property->match_records_against_regexp('/, (.*)/');

    $summary = $property->get_split_summary();
    $this->assertNotEmpty($summary);

    $child_cvterm = factory('chado.cvterm')->create();

    $property->set_child_term($child_cvterm->cvterm_id);

    $property->split_term_by_value_regexp();

    $children = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp', ['biomaterialprop_id'])
      ->condition('type_id', $child_cvterm->cvterm_id)
      ->condition('value', '100 degrees')
      ->execute()->fetchAll();

    $this->assertNotEmpty($children);

    //TODO: test that parent is correct too.


    $parent_gone = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp', ['biomaterialprop_id'])
      ->condition('type_id', $cvterm->cvterm_id)
      ->condition('value', 'four hours, 100 degrees')
      ->execute()->fetchField();

    $this->assertFalse($parent_gone);

    $parent = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp', ['biomaterialprop_id'])
      ->condition('type_id', $cvterm->cvterm_id)
      ->condition('value', 'six days')
      ->execute()->fetchObject();

    $this->assertNotFalse($parent);



  }

  /**
   * Create two properties: one that will be split and produce another of the same type.
   * split should pass, BUT it shouldnt overwrite the existing property terms.
   *
   * @ticket 34
   */
  public function test_split_doesnt_update_if_present(){


    $cv = factory('chado.cv')->create();

    $cvtermA = factory('chado.cvterm')
      ->create(['cv_id' => $cv->cv_id]);
    $cvtermB = factory('chado.cvterm')
      ->create(['cv_id' => $cv->cv_id]);

    $biomaterial = factory('chado.biomaterial')->create();


    $values = [
      'part A@part C',
      'part B',
    ];

    $propA = factory('chado.biomaterialprop')
      ->create([
        'type_id' => $cvtermA->cvterm_id,
        'biomaterial_id' => $biomaterial->biomaterial_id,
        'value' => $values[0],
      ]);

    $propB = factory('chado.biomaterialprop')
      ->create([
        'type_id' => $cvtermB->cvterm_id,
        'biomaterial_id' => $biomaterial->biomaterial_id,
        'value' => $values[1],
      ]);

    //Splitting PropA on @ with the match going to cvtermB should fail.


    $children = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp', ['value', 'type_id', 'rank'])
      ->condition('biomaterial_id', $biomaterial->biomaterial_id)
      ->execute()->fetchAll();

    $property = new Chado_property();
    $tables = $property->set_cvtermprop_search($cvtermA->cvterm_id);
    $props = $property->get_props();

    $qualifiers = $property->match_records_against_regexp('/@(.*)/');

    $summary = $property->get_split_summary();
    $this->assertNotEmpty($summary);

    $property->set_child_term($cvtermB->cvterm_id);

    $property->split_term_by_value_regexp();


    $children = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp', ['value', 'type_id', 'rank'])
      ->condition('biomaterial_id', $biomaterial->biomaterial_id)
      ->execute()->fetchAll();
    //We have a different value so we should now have 3 terms
    $this->assertEquals(3, count($children));

    $children = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp', ['value'])
      ->condition('type_id', $cvtermA->cvterm_id)
      ->execute()->fetchObject();

    $this->assertNotNull($children);

    $this->assertEquals('part A', $children->value);


    $children = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp')
      ->condition('type_id', $cvtermB->cvterm_id)
      ->condition('rank', '1')
      ->execute()->fetchObject();

    $this->assertEquals('part C', $children->value);


    $children = db_select('chado.biomaterialprop', 'bp')
      ->fields('bp')
      ->condition('type_id', $cvtermB->cvterm_id)
      ->condition('rank', '0')
      ->execute()->fetchObject();

    $this->assertEquals('part B', $children->value);
  }



}
