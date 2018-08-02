
## Introduction

Tripal Curator was designed for Tripal sites with lots of legacy data already loaded into Chado.  As FAIR data use has become more important, there is a need to update records loaded with custom, site specific CVterms to CVterms from recognize controlled vocabularies and ontologies.

In the example below, we'll look at upgrading the properties associated with biomaterials loaded using default settings with the [Tripal Analysis Expression module](https://github.com/tripal/tripal_analysis_expression). The **CV USAGE** form will let us see an overview of CV usage by property table.  We can then select a specific CV and reassign terms from there.

**note: some local terms are essential for proper module function. For example, the analysis_type property defines the analysis subtype. Use with care.**  There are no such issues with Biomaterialprops as loaded by the [Tripal Analysis Expression module](https://github.com/tripal/tripal_analysis_expression). 


## Usage

The Edit by CV area is located at `admin/tripal/extension/tripal_curator/CV_usage`.  

![CV usage area](/docs/img/edit_by_cv/cv_usage_table.png)

 * Pick the CV you'd like to re-annotate. You can see which cvs are used in which types of properties. In this example, let's pick the Tripal vocabulary for biomaterials. * Pick a term used we want to change. You will see all values used for that property term. Let's pick temperature.
 
 ![biomaterial cv usage](/docs/img/edit_by_cv/biomaterialcv_page.png)
 
 * We need a new cvterm for temperature. For HWG, we'll look them up in the plant trait ontology
 
 
![biomaterial cv usage](/docs/img/edit_by_cv/ look_up_pto_temp_term.png)
 
 * once you've picked your term, ensure it exists in your site and select it
 
 ![look up term](/docs/img/edit_by_cv/curator_temp_lookup.png)
 curator_temp_lookup
 
 * check which prop table to apply the change to. you might want to only change a specific table if the term is in use multiple places.
 
 ![temp biomaterial](/docs/img/edit_by_cv/temp_biomaterialprop_only.png)
 temp_biomaterialprop_only
 
 * once the term for these properties
 is changed, no more properties use this term.
 
 ![temp is gone](/docs/img/edit_by_cv/temp_is_gone.png)
 
