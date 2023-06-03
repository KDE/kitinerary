/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <memory>

namespace KItinerary {
class ExtractorDocumentNode;
class ExtractorEngine;
class ExtractorResult;
class ScriptExtractor;
class ExtractorScriptEnginePrivate;

/** JavaScript execution environment for KItinerary::ScriptExtractor instances. */
class ExtractorScriptEngine
{
public:
    explicit ExtractorScriptEngine();
    ~ExtractorScriptEngine();
    void setExtractorEngine(ExtractorEngine *engine);

    ExtractorResult execute(const ScriptExtractor *extractor, const ExtractorDocumentNode &node, const ExtractorDocumentNode &triggerNode) const;

private:
    void ensureInitialized();
    std::unique_ptr<ExtractorScriptEnginePrivate> d;
};

}

