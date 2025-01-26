# JavaScript Extractor Scripts

Kitinerary provides a JavaScript-engine for writing extractors to parse and output structured data from tickets and
other travel documents.
This data is later used in many projects to generate useful information for the user.

All extractor scripts are written in JavaScript and [stored in
`/src/lib/scripts`](https://invent.kde.org/pim/kitinerary/-/tree/master/src/lib/scripts).

# How to make your own extractor script

It's highly recommended to use the [KItinerary Workbench](https://invent.kde.org/pim/kitinerary-workbench) to develop
and test your extractor scripts.

It can be installed via Flatpak. For more information, see
the [KItinerary Workbench README](https://invent.kde.org/pim/kitinerary-workbench/-/blob/master/README.md).

## Creating a new extractor script

To create a new extractor script, you need to create a new files in the `$XDG_DATA_DIRS/kitinerary/extractors` (~/
.local/share/kitinerary/extractors`) directory.

> **Note:**: For easier management and later collaboration in Git we recommend linking extractor scripts from the
> directory to a Git repository (`ln -s $(pwd)/src/lib/scripts ~/.local/share/kitinerary/extractors`).

## Script declaration

Kitinerary uses a JSON file to declare the extractor scripts. This file sets filtering rules by which it knows which
extractor to run and defines the script itself.

> **Note:**: Multiple extractors can run on a single document, if more than one extractor outputs valid data it will be
> merged into single output.
> **Note:**: Multiple script declarations can exist for one js extractor file. Usefull if there many types of documents
> but same script can be used to extract data from them.

### Document type

Declare the document type in the JSON file. This is the MIME type of the document, which is used to match the document,
each object representing a single script.

```json
{
  "mimeType": "application/pdf",
  // MIME type of the document (aka. what that document is)
  "filter": [
    {
      ...
    }
  ],
  // Rules by which detect this ticket should be parsed by this script 
  "script": "my-extractor-script.js",
  // Name of the extractor script itself
  "function": "extractTicket"
  // Name of the function in the script that will be called 
}
```

Extractor scripts are run against a document node if all of the following conditions are met:

* The `mimeType` of the script matches that of the node.
* At least one of the extractor `filter` of the script match the node.

### Extractor filters

Extractor filters are evaluated against document nodes. This can be the node the extractor
script wants to process, but also a descendant or ancestor node.

An extractor script filter consists of the following four properties:

```json
{
  "[...]": "[...]",
  "filter": [
    {
      "mimeType": "...",
      // Specifies the type of document you're looking for (e.g., QR code in document, plain text). 
      "field": "...",
      // If it is a complex document (like email with fields), it sets which filed of the document run "match" on. This is ignored for nodes containing basic types such as plain text or binary data.
      "match": "...",
      //a regular expression or exact string we look for.
      "scope": "..."
      //this defines the relation to the node the script should be run on (Current, Parent, Children, Ancestors or Descendants).
    }
    // [...]
  ]
}

```

TODO: Add more information about the `scope` property. what it actually means?

<details>
<summary>Examples</summary>

--- 
Anything attached to an email sent by "booking@example-operator.com". The field matched against here
is the `From` header of the MIME message.

```json
{
  "mimeType": "message/rfc822",
  // Its mime type of email
  "field": "From",
  // We look at fiels "From" in the email, which is the sender
  "match": "^booking@exampl-operator\.com$",
  // We look at exactly "booking@exampl-operator.com" 
  "scope": "Ancestors"
  // TODO: What is this?
}
```

---
Documents containing a barcode of the format "F12345678". Note that the scope here is `Descendants`
rather than `Children` as the direct child nodes tend to be the images containing the barcode.

```json
{
  "mimeType": "text/plain",
  // we look at plain text
  "scope": "Ancestors",
  // TODO: What is this? i dont understand... 
  "match": "^F\d{8}$"
  // We look for exactly "F" followed by 8 digits
}
```

---
Apple Wallet passes issued by "org.kde.travelAgency".

```json
{
  "mimeType": "application/vnd.apple.pkpass",
  // We look at Apple Wallet passes
  "field": "passTypeIdentifier",
  // We look at field "passTypeIdentifier" which is the issuer
  "match": "org.kde.travelAgency",
  "scope": "Current"
  // We look only at this document
}
```

---
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

---
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

---
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

---
**NOT RECOMMENDED** This should be used as a last resort only, as matching against the full PDF document content can be
expensive.

PDF documents containing the string "My Ferry Booking" anywhere.

```json
{
  "mimeType": "application/pdf",
  "field": "text",
  "match": "My Ferry Booking",
  "scope": "Current"
}
```

</details>

## Extractor script

Extractor scripts are run inside a QJSEngine, it is not a full JS environment, and not everything is supported.
There are some additional APIs available to extractor scripts (technical docs can be found
here [KItinerary::JsApi](https://api.kde.org/kdepim/kitinerary/html/namespaceKItinerary_1_1JsApi.html).

### Additional API available to extractor scripts

#### JSON-LD API

API for supporting schema.org output:

* `JsonLd`: factory functions for schema.org objects, date/time parsing, etc

> More: [JsonLd](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1JsApi_1_1JsonLd.html)

<details>
<summary>Examples</summary>

```js

var f = JsonLd.newFlightReservation(); // https://schema.org/FlightReservation
f.reservationFor.departureAirport.name = "KDE Konqi Airport (KDQ)"; // https://schema.org/FlightReservation -> https://schema.org/Flight -> https://schema.org/Place -> https://schema.org/Airport
f.reservationFor.arrivalAirport.name = "KDE Katie City Airport (KDA)";
f.reservationFor.departureTime = JsonLd.toDateTime("08:36 20.02.2025", "hh:mm dd.MM.yyyy", "en");
f.reservationFor.arrivalTime = JsonLd.toDateTime("09:56 20.02.2025", "hh:mm dd.MM.yyyy", "en");
f.reservationFor.airline.iataCode = "KD";
f.reservationFor.flightNumber = "KD 1096";
return f; // Returns the flight reservation object later used by other apps 
```

</details>

#### ByteArray, BitArray, Barcode

API for handling specific types of input data:

* `ByteArray`: functions for dealing with byte-aligned binary data, including decompression, Base64 decoding, Protcol
  Buffer decoding, etc.
* `BitArray`: functions for dealing with non byte-aligned binary data, such as reading numerical data at arbitrary bit
  offsets.
* `Barcode`: functions for manual barcode decoding. This should be rarely needed nowadays, with the extractor engine
  doing this automatically and creating corresponding document nodes.

> More:
> [ByteArray](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1JsApi_1_1ByteArray.html),
> [BitArray](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1JsApi_1_1BitArray.html)
> [Barcode](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1JsApi_1_1Barcode.html)

<details>
<summary>Examples</summary>

```js
const KonqiPersonality = ByteArray.toBase64("Cheerful"); // "Q2hlZXJmdWwK"
const KatieMessage = ByteArray.fromBase64("UmVtZW1iZXIgdG8gdGFrZSBicmVha3MK"); // "Remember to take breaks"

// FIXME: How to use ByteArray?

// Usually not needed, as the extractor engine will create barcode nodes automatically
const QRCode = ImageOfAztecQRCodeNotDecodedByExtractorEngine;
const DecodedAztec = Barcode.decodeAztec(ImageOfAztecQRCodeNotDecodedByExtractorEngine)
z
console.log(DecodedAztec); // ["KDE airlines", "KDE Konqi Airport (KDQ)", "KDE Katie City Airport (KDA)", "20.02.2025", "08:36", "20.02.2025", "09:56", "KD 1096", "magicalstringsoweknowthisticketwasnottamperedwithbyevilwizards"]
```

</details>

#### Extractor API

API for interacting with the extractor engine itself:

* `ExtractorEngine`: this allows to recursively perform extraction.
  This can be useful for elements that need custom decoding in an extractor script first,
  but that contain otherwise generally supported data formats. Standard barcodes encoded
  in URL arguments are such an example.

> More: [ExtractorEngine](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1ExtractorEngine.html)


<details>
<summary>Examples</summary>

```js
const XMLdataIncorreclyInterpretedAsText = "<xml><data>42</data></xml>";
const CorrectlyInterpretedXML = ExtractorEngine.extract(XMLdataIncorreclyInterpretedAsText, "application/xml");

var f = JsonLd.newFlightReservation();
ExtractorEngine.extractPrice("13 EUR", f); // Adds to ticket price
```

</details>

### Objects of a document

#### ExtractorDocumentNode (node)

It's a object that represents a node in the document tree:

* `content`: Value of the node (eg. text, barcode content, etc)
* `childNodes`: List of child of this node, they are also ExtractorDocumentNode objects.
* `mimeType`: MIME type of the node (eg. text/plain, application/pdf, internal/qimage etc)

<details>
<summary>Examples</summary>

```tree
└── application/pdf // Ticket in PDF format
    ├── internal/qimage // Image of the QR code
    │   └── text/plain // Decoded text from the QR code
    └── text/plain // Usually text inside of PDF
```

```js
function main(pdf, node, barcode) {
    cnsole.log(pdf.content) // Automagically extracted PDF content, no need to point at it.
    imageOfQR = node.childNodes[0];
    textFromQR = imageOfQR.childNodes[0].content;
}

```

</details>

#### DocumentNode types

Ticket itself can be in different formats, and each format has its own object:
<details id="PDF - PDF document">
<summary>PDF - PDF document</summary>
PdfDocument is a object that represents a PDF document; it has the following properties:

* `text`: Extracts text from the PDF page. If used on root node, it extracts all text from the PDF.
* `pages`: List of pages in the PDF
* `textInRect`: Extracts text from a given rectangle on the PDF page. Uses normalized coordinates (0-1) in format "Left,
  Top, Right, Bottom".

> More:
> [PdfDocument](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1PdfDocument.html)

  <details>
  <summary>Examples</summary>

  ```js
  // If ticket is in PDF the fist argument is the `PdfPage` object
function main(pdf, node) {
    const allText = content.text; // Extracts all text from the PDF page
    const firstPage = pdf.pages[0].text; // Extracts text from  the first page of the PDF
    const textInRect = pdf.pages[0].textInRect(0, 0, 0.3, 0.25); // "Passanger: Kandalf"
}

```

  </details>
</details>
<details id="Html - HTML document">
<summary>Html - HTML document</summary>

TODO: Actually write this

> More:
> [HtmlDocument](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1HtmlDocument.html)
> [HtmlElement](https://api.kde.org/kdepim/kitinerary/html/classKItinerary_1_1HtmlElement.html)

</details>
<details id="Ical/pkpass - Calendar">
<summary>Ical/pkpass - Calendar</summary>

TODO: Actually write this, i can't find any information about this...
</details>

### Extractor scripts

The script entry point is called with three arguments:

* The first argument is the content of the node that is processed. The data type of that argument
  depends on the node type as described in the document model section above. This is usually
  what extractor script are most concerned with.
* The second argument is the document node being processed (KItinerary::ExtractorDocumentNode, see example under).
  This can be useful to access already extracted results on a node (e.g. coming from generic extraction)
  in order to augment those.
* The third argument is the document node that matched the filter. This can be the same as the second
  argument (for filters with `scope` = Current), but it doesn't have to be. This is most useful when
  triggering on descendant nodes such as barcodes, the content of which will then be incorporated into
  the extraction result by the script.

Output of your JS function should be:

* A JS object following the schema.org ontology (JsonLd) with a single extraction result.
* A JS array containing one or more schema.org/JsonLd objects. Useful if a ticket document has multiple tickets.

> Script errors and empty array is considered as "nothing".

<details>
<summary>Examples</summary>
Let's assume we want to create an extractor script for a railway ticket which comes with a simple
tabular layout for a single leg per page, and contains a QR code with a 10 digit number for each leg.

```
Konqi -> Katie West
Departure: 21 Jun 18:42
Arrival: 21 Jun 23:12

[Big QR code]
```

As a filter we'd use something similar as example 2 above, triggering on the barcode content.

```js
function extractTicket(pdf, node, barcode) {
    // text for the PDF page containing the barcode that triggered this
    const text = pdf.pages[barcode.location].text;

    // empty http://schema.org/TrainReservation object for the result
    let res = JsonLd.newTrainReservation();

    // when using regular expressions, matching on things that don't change in different
    // language variants is usually preferable, but might not always be possible
    // when creating regular expressions consider that various special characters might occur in names
    // of people or locations (in the above example spaces and parenthesis)
    const leg = text.match(/(.*) -> (.*)/); // ["Konqi", "Katie West"]

    // this can throw an error if the regular expression didn't match
    // that's fine though, the script is aborted here and considered not to have any result
    // ie. handling this case explicitly is unnecessary here
    res.reservationFor.departureStation.name = leg[1]; // Konqi
    res.reservationFor.arrivalStation.name = leg[2]; // Katie West

    // date/time parsing can recover missing year numbers from context, if available
    // In our example it would consider the PDF creation time for that, and the resulting
    // date would be the first occurrence of the given day and month following that.
    // https://doc.qt.io/qt-6/qdate.html#fromString-1  
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
function extractBoardingPass(pass, node) {
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

</details>

