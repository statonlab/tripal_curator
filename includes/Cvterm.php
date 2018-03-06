<?php

/*
 * CVterm class
 * Handles retrieving CVterm information
 *
 */


namespace tripal_curator;


class Cvterm {


  private  $cvterm_id;

  public function __construct() {

  }

  public function set_id($id){

    $cvterm = tripal_get_cvterm(array(
      'cvterm_id' => $id,
    ));

    if (!$cvterm){
      tripal_set_message("Error: could not get CVterm for ID = " . $id, TRIPAL_ERROR);
      return null;

    }
    $this->cvterm_id = $cvterm->cvterm_id;

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

    if (!$cvterm){
      tripal_set_message("Error: could not get CVterm for ID = " . $this->cvterm_id, TRIPAL_ERROR);
      return null;

    }
    return $cvterm;
  }


  }