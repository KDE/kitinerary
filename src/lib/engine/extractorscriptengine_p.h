/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORSCRIPTENGINE_P_H
#define KITINERARY_EXTRACTORSCRIPTENGINE_P_H

#include <memory>

namespace KItinerary {
class BarcodeDecoder;
class ExtractorDocumentNode;
class ExtractorResult;
class ScriptExtractor;
class ExtractorScriptEnginePrivate;

/** JavaScript execution environment for KItinerary::ScriptExtractor instances. */
class ExtractorScriptEngine
{
public:
    explicit ExtractorScriptEngine();
    ~ExtractorScriptEngine();
    void setBarcodeDecoder(BarcodeDecoder *barcodeDecoder);

    ExtractorResult execute(const ScriptExtractor *extractor, const ExtractorDocumentNode &node, const ExtractorDocumentNode &triggerNode) const;

private:
    void ensureInitialized();
    std::unique_ptr<ExtractorScriptEnginePrivate> d;
};

}

#endif // KITINERARY_EXTRACTORSCRIPTENGINE_P_H
