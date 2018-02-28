<?php


namespace tripal_curator;


class CV {


  private  $cv_id;



  public function set_id($id){




    $this->cv_id = $id;

  }


  public function set_props($prop_tables){
    $this->prop_tables = $ $prop_tables;

    $term_list = [];

    foreach ($prop_tables as $prop_table ){


    }
    return $term_list;
  }

}