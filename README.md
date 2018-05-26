# KItinerary

A library containing itinerary data model and itinerary extraction code.

## Data Model

This follows the reservation ontology from https://schema.org and Google's extensions to it
(https://developers.google.com/gmail/markup/reference/).

De/serialization is provided via KItinerary::JsonLdDocument.

## Data Extraction

There's a number of ways to extract reservation or itinerary data:

* Structured data in JSON-LD or XML microdata format, from HTML emails,
  provided by KItinerary::StructuredDataExtractor.
* Structured data from IATA bar coded boarding passes (BCBP), provided by
  KItinerary::IataBcbpParser.
* Structured data from UIC 918.3 train ticket passes, provided by
  KItinerary::Uic9183Parser.
* Unstructured data from plain text, HTML or PDF email parts, using
  vendor-specific scripts, provided by KItinerary::ExtractorEngine.
* Unstructured data from Apple Wallet boarding passes, using
  vendor-specific scripts.

## Data Augmentation

Extracted data can be augmented by static knowledge obtained from Wikidata:

Via KItinerary::KnowledgeDb:
* Airport IATA codes, countries, timezones and geo coordinates.
* Train station countries, timezones and geo coordinates.
* Train station lookup by IBNR or Gares & Connexion IDs.
* Country ISO codes, driving side and used power plugs.
