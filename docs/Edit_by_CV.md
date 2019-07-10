
## Introduction

Tripal Curator was designed for Tripal sites with lots of legacy data already loaded into Chado.  As FAIR data use has become more important, there is a need to update records loaded with custom, site specific CVterms to CVterms from recognize controlled vocabularies and ontologies.

In the example below, we'll look at upgrading the properties associated with biomaterials loaded using default settings with the [Tripal Analysis Expression module](https://github.com/tripal/tripal_analysis_expression). The **CV USAGE** form will let us see an overview of CV usage by property table.  We can then select a specific CV and reassign terms from there.

**note: some local terms are essential for proper module function. For example, the analysis_type property defines the analysis subtype. Use with care.**  There are no such issues with Biomaterialprops as loaded by the [Tripal Analysis Expression module](https://github.com/tripal/tripal_analysis_expression). 

## Usage

The Edit by CV area is located at `admin/tripal/extension/tripal_curator/CV_usage`.  

This page lists every Chado prop table in use on your site.  It then breaks down which properties use which CVs for the `type_id` column of each property.

![CV usage area](/docs/img/edit_by_cv/cv_usage_table.png)

In addition, this table tells us what **values** are asigned to properties using this term.  This can give you an idea if the property needs to be split into multiple properties, or if you have multiple property types providing similar information.

Pick the CV you'd like to re-annotate.  In this example, let's pick the Tripal vocabulary for biomaterials.
 
 ![biomaterial cv usage](/docs/img/edit_by_cv/biomaterialcv_page.png)
 
Pick a term used we want to change. You will see all values used for that property term. Let's pick temperature.
 
We need a new cvterm for temperature. For our site, we'll look them up in the plant trait ontology.  In general, I suggest the [EBI Ontology Lookup Service](https://www.ebi.ac.uk/ols/index) for finding terms.

![biomaterial cv usage](/docs/img/edit_by_cv/look_up_PTO_temp_term.png)
 
Once you've picked your term, ensure it exists in your site and select it as the destination term for this property.
 
 ![look up term](/docs/img/edit_by_cv/curator_temp_lookup.png)
 
 Check which prop table to apply the change to. you might want to only change a specific table if the term is in use multiple places.
 
 Submitting the form remaps all `biomaterialprop` entries with type set to temperature from the `biomaterial_property` CV to the plant trait ontology term.
 
 ![temp biomaterial](/docs/img/edit_by_cv/temp_biomaterialprop_only.png)
 
 Once the term for these properties
 is changed, no more properties use this term.  We can move on to mapping additional terms.
 
 ![temp is gone](/docs/img/edit_by_cv/temp_is_gone.png)
 
