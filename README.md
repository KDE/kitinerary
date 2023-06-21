# Itinerary data extraction engine

The itinerary data extraction engine extracts travel-related information from input in various forms,
from PDF documents to ticket barcodes, from emails to calendar events, and provides that in a machine-readable way.

## Users

* [KDE Itinerary](https://apps.kde.org/itinerary)
* [KMail](https://kontact.kde.org/components/kmail/) (via the Itinerary plug-in)
* [Nextcloud Mail](https://github.com/nextcloud/mail)

## Architecture

For linked class names read this in [the API docs](https://api.kde.org/kdepim/kitinerary/html/index.html).

### Data model

This follows the reservation ontology from https://schema.org and Google's extensions to it
(https://developers.google.com/gmail/markup/reference/).

De/serialization is provided via KItinerary::JsonLdDocument.

### Document model

Input data is transformed into a tree of document nodes (KItinerary::ExtractorDocumentNode).
This allows handling of arbitrarily nested data, such as an email with a PDF attached to it
which contains an image that contains a barcode with an UIC 918.3 ticket container, without
extractors having to consider all possible combinations.

A document node consists of a MIME type and its corresponding data, and potentially a number
of child nodes.

Data extraction is then performed on that document tree starting at the leaf nodes, with results
propagating upwards towards the root node.

Supported types of data are listed below. Additional data formats can be added via
KItinerary::ExtractorDocumentProcessor and KItinerary::ExtractorDocumentNodeFactory.

#### Generic document formats

* PDF documents, represented as KItinerary::PdfDocument.
* Emails, represented as KMime::Message.
* Apple Wallet passes, represented as KPkPass::Pass.
* iCal calendars and iCal calendar event, represented as KCalendarCore::Calendar and KCalendarCore::Event.
* HTML and XML documents, represented has KItinerary::HtmlDocument.

#### Specialized ticket barcode formats

* UIC 918.3/918.9 ticket barcodes, represented as KItinerary::Uic9183Parser.
* European Railway Agency (ERA) FCB ticket barcodes, represented as KItinerary::Fcb::UicRailTicketData.
* European Railway Agency (ERA) SSB ticket barcodes, represented as KItinerary::SSBv1Ticket,
  KItinerary::SSBv2Ticket and KItinerary::SSBv3Ticket.
* IATA boarding pass barcodes, represented as KItinerary::IataBcbp.
* VDV eTicket barcodes, represented as KItinerary::VdvTicket.

#### Technical data types

These are primarily needed for internal use.

* Images, represented as QImage.
* Apple property lists (plist), represented as KItinerary::PListReader.
* HTTP responses, represented as KItineary::HTTPResponse.

#### Generic data types

These capture everything not handled above.

* JSON, represented as QJsonArray.
* Plain textual data, represented as a QString.
* Arbitrary binary data, represented as a QByteArray.

### Data extraction

Data extraction is performed on the document tree starting at the leaf nodes, with results
propagating upwards towards the root node. This means that results from child nodes are available
to the extraction process, and can be extended/augmented there for example.

The entry point for data extraction is KItinerary::ExtractorEngine.

There's a number of built-in generic extractors for the following cases:
* The various ticket barcode types (IATA, UIC 918.3/9, ERA FCB, ERA SSB).
* Structured data in JSON-LD or XML microdata format included in HTML documents or iCal events.
* PDF flight boarding passes.
* Apple Wallet passes for flights, trains or events.
* iCal calendar events (depends on KItinerary::ExtractorEngine::ExtractGenericIcalEvents).

To cover anything not handled by this, there are vendor-specific extractor scripts. Those
can produce complete results or merely fix or augment what the generic extraction has produced.

Extractor scripts consist of two basic parts, the filter defining when it should be triggered
and the script itself (see KItinerary::ScriptExtractor). This is necessary as running all extractor
scripts against a given input data would be too expensive. Filters therefore don't need to be perfect
(noticing in the script it triggered on the wrong document is fine), but rather fast.

### Data augmentation

Extracted data can be augmented by static knowledge obtained from Wikidata:

Via KItinerary::KnowledgeDb:
* Airport IATA codes, countries, timezones and geo coordinates.
* Train station countries, timezones and geo coordinates.
* Train station lookup by UIC, IBNR, SNCF, VR or Indian Railway station identifiers.
* Country ISO codes, driving side and used power plugs.
* Timezone and country lookup from a geo coordinate.


## Creating extractor scripts

### Extractor filters

Extractor filters are evaluated against document nodes. This can be the node the extractor
script wants to process, but also a descendant or ancestor node.

An extractor script filter consists of the following four properties:
* `m̀imeType`: the type of the node to match
* `field`: the property of the node content to match. This is ignored for nodes containing
  basic types such as plain text or binary data.
* `match`: a regular expression
* `scope`: this defines the relation to the node the script should be run on (Current, Parent,
  Children, Ancestors or Descendants).

#### Examples

Anything attached to an email send by "booking@example-operator.com". The field we match against here
is the `From` header of the MIME message.

```json
{
    "mimeType": "message/rfc822",
    "field": "From",
    "match": "^booking@exampl-operator.com$",
    "scope": "Ancestors"
}
```

Documents containing a barcode of the format "FNNNNNNNN". Note that the scope here is `Descendants`
ather than `Children` as the direct child nodes tend to be the images containing the barcode.

```json
{
    "mimeType": "text/plain",
    "match": "^F\d{8}$",
    "scope": "Ancestors"
}
```

PDF documents containing the string "My Ferry Booking" anywhere. This should be used as a last resort
only, as matching against the full PDF document content can be expensive. An imprecise trigger on a
barcode is preferable to this.

```json
{
    "mimeType": "application/pdf",
    "field": "text",
    "match": "My Ferry Booking",
    "scope": "Current"
}
```

Apple Wallet passes issued by "org.kde.travelAgency".

```json
{
    "mimeType": "application/vnd.apple.pkpass",
    "field": "passTypeIdentifier",
    "match": "org.kde.travelAgency",
    "scope": "Current"
}
```

iCal events with an organizer email address of the "kde.org" domain. Note that the field here accesses
a property of a property. This works at arbitrary depth, as long as the corresponding types are
introspectable by Qt.

```json
{
    "mimeType": "internal/event",
    "field": "organizer.email",
    "match": "@kde.org$",
    "scope": "Current"
}
```

A (PDF) document containing an IATA boarding pass barcode of the airline "AB". Triggering
vendor-specific UIC or ERA railway tickets can be done very similarly, matching on the corresponding
carrier ids.

```json
{
    "mimeType": "internal/iata-bcbp",
    "field": "operatingCarrierDesignator",
    "match": "AB",
    "scope": "Descendants"
}
```

A node that has already existing results containing a reservation from "My Transport Operator".
This is useful for scripts that want to augment or fix schema.org annotation already provided by
the source. Note that the mimeType "application/ld+json" is special here as it doesn't only trigger
on the document node content itself, but also matches against the result of nodes of any type.

```json
{
    "mimeType": "application/ld+json",
    "field": "reservationFor.provider.name",
    "match": "My Transport Operator",
    "scope": "Current"
}
```

### Extractor scripts

Extractor scripts are defined by the following properties:
* `script`: The name of the script file.
* `function`: The name of the JS function that is called as the entry point into the script.
* `mimeType`: The MIME type the script can handle.
* `filter`: A list of extractor filters as described above.

Extractor scripts are run against a document node if all of the following conditions are met:
* The `mimeType` of the script matches that of the node.
* At lesat one of the extractor `filter` of the script match the node.

The script entry point is called with three arguments (this being JS, some of those can be omitted
by the script and are then silently ignored):
* The first argument is the content of the node that is processed. The data type of that argument
depends on the node type as described in the document model section above. This is usually
what extractor script are most concerned with.
* The second argument is the document node being processed (see KItinerary::ExtractorDocumentNode).
This can be useful to access already extracted results on a node (e.g. coming from generic extraction)
in order to augment those.
* The third argument is the document node that matched the filter. This can be the same as the second
argument (for filters with `scope` = Current), but it doesn't have to be. This is most useful when
triggering on descendant nodes such as barcodes, the content of which will then be incorporated into
the extraction result by the script.

The script entry point function is expected to return one of the following:
* A JS object following the schema.org ontology with a single extraction result.
* A JS array containing one or more such objects.
* Anything else (including empty arrays and script errors) are considered an empty result.

### Extractor scripts runtime environment

Extractor scripts are run inside a QJSEngine, ie. that's the JS subset to work with.
There is some additional API available to extractor scripts (see the KItinerary::JsApi namespace).

API for supporting schema.org output:
* KItinerary::JsApi::JsonLd: factory functions for schema.org objects, date/time parsing, etc

API for handling specific types of input data:
* KItinerary::JsApi::ByteArray: functions for dealing with byte-aligned binary data,
  including decompression, Base64 decoding, Protcol Buffer decoding, etc.
* KItinerary::JsApi::BitArray: functions for dealing with non byte-aligned binary data,
  such as reading numerical data at arbitrary bit offsets.
* KItinerary::JsApi::Barcode: functions for manual barcode decoding. This should be rarely
  needed nowadays, with the extractor engine doing this automatically and creating corresponding
  document nodes.

API for interacting with the extractor engine itself:
* KItinerary::JsApi::ExtractorEngine: this allows to recursively perform extraction.
  This can be useful for elements that need custom decoding in an extractor script first,
  but that contain otherwise generally supported data formats. Standard barcodes encoded
  in URL arguments are such an example.

### Script development

[KItineary Workbench](https://commits.kde.org/kitinerary-workbench) allows interactive development
of extractor scripts.

### Examples

Let's assume we want to create an extractor script for a railway ticket which comes with a simple
tabular layout for a single leg per page, and contains a QR code with a 10 digit number for each leg.

```
City A -> City B (Central Station)
Departure: 21 Jun 18:42
Arrival: 21 Jun 23:12
...
```

As a filter we'd use something similar as example 2 above, triggering on the barcode content.

```js
function extractTicket(pdf, node, barcode)
{
    // text for the PDF page containing the barcode that triggered this
    const text = pdf.pages[barcode.location].text;

    // empty http://schema.org/TrainReservation object for the result
    let res = JsonLd.newTrainReservation();

    // when using regular expressions, matching on things that don't change in different
    // language variants is usually preferable, but might not always be possible
    // when creating regular expressions consider that various special characters might occur in names
    // of people or locations (in the above example spaces and parenthesis)
    const leg = text.match(/(.*) -> (.*)/);

    // this can throw an error if the regular expression didn't match
    // that's fine though, the script is aborted here and considered not to have any result
    // ie. handling this case explicitly is unnecessary here
    res.reservationFor.departureStation.name = leg[1];
    res.reservationFor.arrivalStation.name = leg[2];

    // date/time parsing can recover missing year numbers from context, if available
    // In our example it would consider the PDF creation time for that, and the resulting
    // date would be the first occurrence of the given day and month following that.
    res.reservationFor.departureTime = JsonLd.toDateTime(text.match(/Departure: (.*)/)[1], 'dd MMM hh:mm', 'en');
    // for supporting different language formats, both the format string and the locale
    // argument can be lists. All combinations are then tried until one yields a valid result.
    res.reservationFor.arrivalTime = JsonLd.toDateTime(text.match(/(?:Arrival|Arrivé|Ankunft): (.*)/)[1],
        ['dd MMM hh:mm', 'dd MMM hh.mm'], ['en', 'fr', 'de']);

    // the node that triggered this script (the barcode) can be accessed and integrated into the result
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;

    return res;
}
```

## Contributing

Join us in the [KDE Itinerary Matrix channel](https://matrix.to/#/#itinerary:kde.org)!
