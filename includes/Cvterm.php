<?php

/*
 * CVterm class
 * Handles retrieving CVterm information
 *
 */


namespace tripal_curator;


class Cvterm {


  private  $cvterm_id;
  private  $cvterm_cv;
  private  $cvterm_cv_name;
  private  $cvterm_db;
  private  $cvterm_accession;

  public function __construct() {

  }

  /**
   * Returns the full CVterm entry
   *
   * @return array|mixed
   */
  public function get_full(){

    $cvterm = tripal_get_cvterm(array(
      'cvterm_id' => $this->cvterm_id,
    ));
    return $cvterm;
  }


  }