<?php

/**
 * This form is for the CONVERSION and SPLITTING of values
 *
 */

/**
 *
 * @param $form
 * @param $form_state
 * @param $type_id
 *
 * @return mixed
 */
function tripal_curator_assign_cvalue_form($form, &$form_state) {


  $markup = "<p>The Tripal Curator Cvalue mapper lets you easily batch assign CVterms to the value of your properties, where supported.  You can even split properties into multiple Properties!</p>";
  $form['welcome'] = [
    '#markup' => $markup
  ];

  //Step one: Search for *VALUE* entries

  //Step two: Determine Cvalue

  //Step three: Determine if splitting

  //Step four: Split as necessary

  //Step five: Convert



return $form;

}