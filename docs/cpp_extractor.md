## Using the extractor engine

### C++ API

Using the C++ API is the most flexible and efficient way to use this. This consists of three steps:

- Extraction: This will attempt to find relevant information in the given input documents, its
  output however can still contain duplicate or invalid results.
  There are some options to customize this step, e.g. trading more expensive image processing against
  finding more results, depending on how certain you are the input data is going to contain such data.
  See KItinerary::ExtractorEngine.
- Post-processing: This step merges duplicate or split results, but its output can still contain
  invalid elements.
  The main way to customize this step is in what you feed into it. For best results this should be all
  extractor results that can possibly contain information for a specific incident.
  See KItinerary::ExtractorPostprocessor.
- Validation: This will remove and remaining incomplete or invalid results, or results of undesired types.
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
