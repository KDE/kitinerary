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

The data model used in here follows the [schema.org](https://schema.org) ontology, and for historic
reasons some of [Google's extensions](https://developers.google.com/gmail/markup/reference/) to it.

Various QML-compatible value classes based on that can be found in the `src/lib/datatypes` sub-directory.
Those do not implement the schema.org ontology one to one though, but focus on a subset relevant
for the current consumers. Any avoidable complexity of the ontology is omitted, which mainly
shows in a significantly flattened inheritance hierarchy, and stricter property types. This
is done to make data processing and display easier.

There is one notable extension to the schema.org model, all date/time values support
explicit IANA timezone identifiers, something that JSON cannot model out of the box.

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
* HTML and XML documents, represented as KItinerary::HtmlDocument.

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
* ActivityPub events and places.

To cover anything not handled by this, there are vendor-specific extractor scripts. Those
can produce complete results or merely fix or augment what the generic extraction has produced.

Extractor scripts consist of two basic parts, the filter defining when it should be triggered
and the script itself (see KItinerary::ScriptExtractor). This is necessary as running all extractor
scripts against a given input data would be too expensive. Filters therefore don't need to be perfect
(noticing in the script it triggered on the wrong document is fine), but rather fast.

### Data post-processing and augmentation

A number of additional processing steps are applied to extracted data
(see KItineary::ExtractorPostProcessor).

#### Normalization

* Simplify whitespaces in human-readable strings.
* Separate postal codes in addresses.
* Remove name prefixes.
* Convert human-readable country names into ISO 3166-1 alpha 2 country codes.
* Apply timezones to date/time values.
* Identify IATA airport codes based on airport names.

#### Augmentation

* Geographic coordinates based on IATA airport codes as well as a number of
  train station code.
* Timezones based on geographic coordinates, or where sufficiently unique
  country/region information.
* Countries and regions based on geographic coordinates.
* Countries based on international phone numbers (needs libphonenumbers).

Most of this data is obtained from [OpenStreetMap](https://openstreetmap.org)
and [Wikidata](https://wikidata.org) and provided as part of this library. No
online operations are performed during extraction or post-processing.

#### Merging

If the result set contains multiple elements, merging elements referring
to the same incidence is attempted. Two cases are considered:

* Elements that are considered to refer to exactly the same incidence
  are folded into one.
* An element referring to a location change from A to B and two elements
  referring to a location change from A to C and C to B are considered
  to refer to the same trip, with the first one providing a lower level
  of detail. The first element is folded into the other two in that case.

#### Validation

In the final step all results are checked for containing a bare minimum of information
(e.g. time and name for an event), and for being self-consistent (e.g. start time before end time).
Invalid results are discarded. See KItinerary::ExtractorValidator.


## Creating extractor scripts

Extractor scripts are searched for in two locations:
* In the file system at `$XDG_DATA_DIRS/kitinerary/extractors`.
* Compiled into the binary at `:/org.kde.pim/kitinerary/extractors`.

Those locations are searched for JSON files containing one or more extractor script
declarations.

```json
{
    "mimeType": "application/pdf",
    "filter": [ { ... } ],
    "script": "my-extractor-script.js",
    "function": "extractTicket"
}
```

The above example shows a single script declarations, for declaring multiple scripts in one
file this can also be a JSON array of such objects. The individual fields are documented below.

### Extractor filters

Extractor filters are evaluated against document nodes. This can be the node the extractor
script wants to process, but also a descendant or ancestor node.

An extractor script filter consists of the following four properties:
* `mimeType`: the type of the node to match
* `field`: the property of the node content to match. This is ignored for nodes containing
  basic types such as plain text or binary data.
* `match`: a regular expression
* `scope`: this defines the relation to the node the script should be run on (Current, Parent,
  Children, Ancestors or Descendants).

#### Examples

Anything attached to an email sent by "booking@example-operator.com". The field matched against here
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
rather than `Children` as the direct child nodes tend to be the images containing the barcode.

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
* At least one of the extractor `filter` of the script match the node.

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

Extractor scripts are run inside a QJSEngine, i.e. that's the JS subset to work with.
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

[KItinerary Workbench](https://commits.kde.org/kitinerary-workbench) allows interactive development
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

The above example produces and entirely new result. Another common case are scripts that
merely augment an existing result. Let's assume an Apple Wallet pass for a flight, the
automatically extracted result is correct but misses the boarding group. The filter for
this would be similar to example 4 above, triggering on the pass issuer.

```js
// unused arguments can be omitted
function extractBoardingPass(pass, node)
{
    // use the existing result as a starting point
    // generally this can be more than one, but specific types of documents
    // might only produce a deterministic amount (like 1 in this case).
    let res = node.result[0];

    // modify the result as necessary
    res.boardingGroup = pass.field["group"].label;

    // returning a result here will replace the existing results for this node
    return res;
}
```

A large number of real-world examples can also be found in the `src/lib/scripts` folder of the source code
or browsed [here](https://invent.kde.org/pim/kitinerary/-/tree/master/src/lib/scripts).

## Using the extractor engine

### C++ API

Using the C++ API is the most flexible and efficient way to use this. This consists of three steps:
* Extraction: This will attempt to find relevant information in the given input documents, its
  output however can still contain duplicate or invalid results.
  There are some options to customize this step, e.g. trading more expensive image processing against
  finding more results, depending on how certain you are the input data is going to contain such data.
  See KItinerary::ExtractorEngine.
* Post-processing: This step merges duplicate or split results, but its output can still contain
  invalid elements.
  The main way to customize this step is in what you feed into it. For best results this should be all
  extractor results that can possibly contain information for a specific incident.
  See KItinerary::ExtractorPostprocessor.
* Validation: This will remove and remaining incomplete or invalid results, or results of undesired types.
  For this step you typically want to set the set of types your application can handle. Letting incomplete
  results pass can be useful if you do have an existing set of data you want to apply those too.
  See KItineary::ExtractorValidator.

Example:
```c++
using namespace KItinerary;

// Create an instance of the extractor engine
// use engine.setHints(...) to control its behavior
ExtractorEngine engine;

// feed raw data into the extractor engine
// passing a file name or MIME type additional to the data is optional
// but can help with identifying the type of data passed in
// should you already have data in decoded form, see engine.setContent() instead
QFile f("my-document.pdf");
f.open(QFile::ReadOnly);
engine.setData(f.readAll(), f.fileName());

// perform the extraction
const auto extractedData = engine.extract();

// post process the extracted result
ExtractorPostprocessor postproc;

// ExtractorPostprocessor::process() can be called multiple times
// to accumulate a single merged result set
postproc.process(extractedData);
auto result = postproc.result();

// select the type of data you can consume
ExtractorValidator validator;
validator.setAcceptedTypes<TrainReservation, BusReservation>();
validator.setAcceptOnlyCompleteElements(true);

// remove invalid results
result.erase(std::remove_if(result.begin(), result.end(), [&validator](const auto &r) {
    return !validator.isValidElement(r);
}), result.end());
```

### Command line extractor

In cases where integrating with the C++ API isn't possible or desirable, there's also a command
line interface to this, `kitinerary-extractor`.

This reads input data from stdin and outputs schema.org JSON with the results.

For easier deployment, the command line extractor can also be built entirely statically. This
is available directly from the Gitlab CI/CD pipeline on demand. Nightly Flatpak builds are
also available from KDE's nightly Flatpak repository:

```
flatpak remote-add --if-not-exists kdeapps --from https://distribute.kde.org/kdeapps.flatpakrepo
flatpak install org.kde.kitinerary-extractor
```

## Contributing

Contribution of new extractor scripts as well as improvements to the extractor engine are very welcome,
preferably as merge request for this repository.

Another way to contribute is by donating sample data. Unlike similar proprietary solutions our data
extraction runs entirely on your device, so we never get to see user documents and thus rely on donated
material to test and improve the extractor.

Samples can be sent to vkrause@kde.org and will not be published. Anything vaguely looking like a
train, bus, boat, flight, rental car, hotel, event or restaurant bookings/tickets/confirmations/cancellation/etc
is relevant, even when they are seemingly already extracted correctly (in many cases there are non-obvious details
we don't cover yet correctly). If possible, please provide material in its original unaltered form,
for emails the easiest way is "Forward As Attachment", inline forwarding can destroy relevant details.

Feel free to join us in the [KDE Itinerary Matrix channel](https://matrix.to/#/#itinerary:kde.org)!
