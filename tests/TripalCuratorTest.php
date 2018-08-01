<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;
use StatonLab\TripalTestSuite\DBTransaction;

class TripalCuratorTest extends TripalTestCase

{
  use DBTransaction;

  public function test_property_tables_returns_prop_tables_only(){
    $tables = tripal_curator_get_property_tables();
    $this->assertNotEmpty($tables);

    $this->assertContains('biomaterialprop', $tables);
    $this->assertNotContains('featureprop_pub', $tables);

  }

  public function test_stringreplace_finds_basetable_from_proptable(){
    $string="biomaterialprop";
    $expected = "biomaterial";
    $new_string = tripal_curator_str_lreplace("prop", "", $string);

    $this->assertEquals($expected, $new_string);

  }

  public function test_fetch_props_for_prop_table_finds_properties_if_prop_table(){
    $table = "cvtermprop";
    $props = tripal_curator_fetch_props_for_prop_table($table);
    $this->assertNotEmpty($props);
    $table = "assay";
    $props = tripal_curator_fetch_props_for_prop_table($table);
    $this->assertNull($props);
  }


  public function test_tripal_curator_get_property_tables_with_cvalues(){

    $this->assertTrue(db_field_exists(tripal_curator_chadofy("biomaterialprop"), 'cvalue_id'), "cvalue_id not set up on Biomaterialprop");  //Need to add this column if not present.

    $list = tripal_curator_get_property_tables_with_cvalues();
    $this->assertNotEmpty($list);
  }
}