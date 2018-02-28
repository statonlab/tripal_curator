<?php

/**
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */

function tripal_curator_add_property_form($form, &$form_state) {
  //assumption: we're getting these as an argument
  $entities = [];

  $markup = "<h3>Add/Modify Properties</h3><br>This page will allow you to add properties to your selected entities.";
  $form['welcome'] = [
    '#markup' => $markup
  ];

  $form['transaction_type'] = [
    '#type' => 'radio',
    '#options' => ['Add', 'Edit', 'Remove']
  ];

  $type = null;

if (isset($form_state['values'] ) && $form_state['values']['transaction_type']){
  $type = $form_state['values']['transaction_type'];

  }
}