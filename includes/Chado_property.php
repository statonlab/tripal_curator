<?php

namespace tripal_curator;


class Chado_property {

  private static $cvterm_id;

  public static $prop_table;

  public static $value;

  public static $cvalue;



  public function __construct($cvterm_id) {

    $this->cvterm_id = $cvterm_id;

  }


}