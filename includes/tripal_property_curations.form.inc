<?php

/**
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */

function tripal_property_curations_form($form, &$form_state) {

  $markup = "<h3>Tripal Chado Property Curations</h3>
    <p>Welcome to the Tripal Property Curator!</p><p>
    For more information on how to use this page, please see the github repo.
</p>
";

  $form['welcome'] = [
    '#markup' => $markup
  ];



  return $form;
}