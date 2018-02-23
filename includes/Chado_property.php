<?php

namespace tripal_curator;


class Chado_property {

  public static $cvterm;

  public static $prop_table;

  public static $value;

  public static $cvalue;


  private static $property_record_id;

  public function __construct($property_record_id) {

    $this->property_record_id = $property_record_id;

  }


}