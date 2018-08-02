
Chado 1.4 will support the `cvalue_id` column.  In Chado 1.3, properties used a cvterm for the `type_id` and plaintext for the `value`.  This means that the idea of "Color = Green" was represented by a property record where the `type_id` would a cvterm, for example, [the PATO term for color](https://www.ebi.ac.uk/ols/ontologies/pato/terms?iri=http%3A%2F%2Fpurl.obolibrary.org%2Fobo%2FPATO_0000014).  Green would be the raw text value.  Cvalues allow you to also assign a cvterm to the value (in this case, green, such as the [PATO term for green](https://www.ebi.ac.uk/ols/ontologies/pato/terms?iri=http%3A%2F%2Fpurl.obolibrary.org%2Fobo%2FPATO_0000320)).

This form allows you to easily find properties and assign new values or cvalues.

## Search for properties

You can search for properties using their:

* Text value
* Existing Cvalue
* NULL Cvalue

### Text value
Coming soon.

### Existing Cvalue

Coming soon.

### NULL Cvalue

Coming soon.