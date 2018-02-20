[![Build Status](https://travis-ci.org/statonlab/tripal_property_curations.svg?branch=master)](https://travis-ci.org/statonlab/tripal_property_curations)

# Overview

*Tripal Property Curations* is a Toolbox for curating Chado Properties.

It is under development and not suitable for use yet.


### Intended Features

* Easily change properties from "Bad" ontologies to good ones 
  - local CVs (tripal_analysis_expression) to Plant Trait Ontology
  
* Easily annotate properties with cvalue_id's (IE, use CVterms to annotate properties where previously they had free text property values)


# Contributing

### Development

For now we manually define the Drupal root in the individual test.  Make sure that your root in `test/tripal_property_curations.test` is set to the local path for development and the docker path for passing Travis.