/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "extractorengine.h"
#include "extractorcontext.h"
#include "extractorrule.h"
#include "semantic_debug.h"

ExtractorEngine::ExtractorEngine() = default;
ExtractorEngine::~ExtractorEngine() = default;

void ExtractorEngine::setExtractor(const Extractor *extractor)
{
    m_extractor = extractor;
}

const QString &ExtractorEngine::text() const
{
    return m_text;
}

void ExtractorEngine::setText(const QString &text)
{
    m_text = text;
}

QJsonArray ExtractorEngine::extract()
{
    if (!m_extractor || m_text.isEmpty()) {
        return {};
    }

    qCDebug(SEMANTIC_LOG) << m_text << m_text.size();
    ExtractorContext context(this);
    context.setRules(m_extractor->rules());
    executeContext(&context);
    return m_result;
}

static bool isEmptyObject(const QJsonObject &obj)
{
    return obj.size() <= 1 && obj.contains(QLatin1String("@type"));
}

ExtractorEngine::Result ExtractorEngine::executeContext(ExtractorContext *context)
{
    while (!context->rules().isEmpty()) {
        QVector<ExtractorRule *> repeatingRules;
        for (auto it = context->rules().begin(); it != context->rules().end(); ++it) {
            if (!(*it)->match(context)) {
                continue;
            }

            qCDebug(SEMANTIC_LOG) << (*it)->ruleType() << (*it)->dataType() << (*it)->name();
            ExtractorContext subContext(this, context);
            subContext.setRules((*it)->rules());
            subContext.setOffset(context->offset());

            switch ((*it)->ruleType()) {
                case ExtractorRule::Class:
                    subContext.setProperty(QLatin1String("@type"), (*it)->dataType());
                    break;
                case ExtractorRule::Break:
                    return Result::Break;
                default:
                    break;
            }

            const auto subResult = executeContext(&subContext);
            if (subResult == Result::Break) {
                return (*it)->repeats() ? Result::Return : Result::Break;
            }

            if ((*it)->ruleType() == ExtractorRule::Class && !isEmptyObject(subContext.object())) {
                if ((*it)->name().isEmpty()) {
                    m_result.push_back(subContext.object());
                } else {
                    context->setProperty((*it)->name(), subContext.object());
                }
            }
            context->setOffset(subContext.offset());

            if ((*it)->repeats()) {
                repeatingRules.push_back(*it);
            }
        }
        context->setRules(repeatingRules);
    }

    return Result::Return;
}
