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

#### Extractor filters

Extractor filters are evaluated against document nodes. This can be the node the extractor
script wants to process, but also a descendant or ancestor node.

An extractor script filter consists of the following four properties:
* `m̀imeType`: the type of the node to match
* `field`: the property of the node content to match. This is ignored for nodes containing
  basic types such as plain text or binary data.
* `match`: a regular expression
* `scope`: this defines the relation to the node the script should be run on (Current, Parent,
  Children, Ancestors or Descendants).

#### Script development

[KItineary Workbench](https://commits.kde.org/kitinerary-workbench) allows interactive development
of extractor scripts.

### Data augmentation

Extracted data can be augmented by static knowledge obtained from Wikidata:

Via KItinerary::KnowledgeDb:
* Airport IATA codes, countries, timezones and geo coordinates.
* Train station countries, timezones and geo coordinates.
* Train station lookup by UIC, IBNR, SNCF, VR or Indian Railway station identifiers.
* Country ISO codes, driving side and used power plugs.
* Timezone and country lookup from a geo coordinate.


## Contributing

Join us in the [KDE Itinerary Matrix channel](https://matrix.to/#/#itinerary:kde.org)!
