<?php

/**
 *
 */
function tripal_curator_split_prop_form($form, &$form_state, $cvterm_id) {

  $term_object = new \tripal_curator\Cvterm();

  $box = provide_summary_box($cvterm_id, $term_object);


  $form['box'] = $box;


  return $form;

}