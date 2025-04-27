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
which contains an image that contains a barcode with an UIC ticket barcode container, without
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

* UIC 918.3/918.9 ticket barcode containers, represented as KItinerary::Uic9183Parser.
* UIC DOSIPAS ticket barcode containers, represented by KItineary::Dosipas.
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
* The various ticket barcode types (IATA, UIC 918.3/9, DOSIPAS, ERA FCB, ERA SSB).
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

## Creating new extractors

### JavaScript 

Documentation and examples for creating new extractor scripts can be found in the [/docs/js_extractor.md](docs/js_extractor.md) file.

### C++ API

Documentation and examples for creating new extractor scripts can be found in the [/docs/cpp_extractor.md](docs/cpp_extractor.md) file.

### Command line extractor

In cases where integrating with the C++ API isn't possible or desirable, there's also a command
line interface to this, `kitinerary-extractor`.

This reads input data from stdin and outputs schema.org JSON with the results.

For easier deployment, the command line extractor can also be built entirely statically. This
is available directly from the Gitlab CI/CD pipeline on demand. Nightly Flatpak builds are
also available from KDE's nightly Flatpak repository:

```
flatpak install https://cdn.kde.org/flatpak/kitinerary-extractor-nightly/org.kde.kitinerary-extractor.flatpakref
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
