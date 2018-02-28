[![Build Status](https://travis-ci.org/statonlab/tripal_curator.svg?branch=master)](https://travis-ci.org/statonlab/tripal_curator)

# Overview

*Tripal Property Curations* is a Toolbox for curating Chado Properties.

It is under development and not suitable for use yet.


![the curator](/docs/tripal_curator.png)

### Existing Features

#### CV usage overview

The CVterm usage table can be viewed at `/admin/tripal/extension/tripal_curator/CV_usage`.  From here you can see 
* What CVs are used by your property tables
* What property tables use what CVs
* How many entities utilize the CVs

You can then visit an individual CV to view the CVterms from that CV in your prop tables, and to remap them.

#### Property type_id CVterm remapping

Remapping properties is easily done via the Chado Property field on an entity's page.  However, you may have thousands or hundreds of thousands of entries that all tagged with a property that you want to remap.  Visit the Property type CVterm mapper to remap it to a more informative CVterm.



### Intended Features

* Easily change properties from "Bad" ontologies to good ones 
  - local CVs (tripal_analysis_expression) to Plant Trait Ontology
  
* Easily annotate properties with `cvalue_id`'s (IE, use CVterms to annotate properties where previously they had free text property values)

* Split property cvalues into multiple terms for "compound properties"

* Autosuggest matching property terms

# Contributing

### Development

For now we manually define the Drupal root in the individual test.  Make sure that your root in `test/tripal_curator.test` is set to the local path for development and the docker path for passing Travis.