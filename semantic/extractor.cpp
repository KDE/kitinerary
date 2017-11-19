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

#include "extractor.h"
#include "extractorrule.h"
#include "semantic_debug.h"

#include <QFile>
#include <QXmlStreamReader>

#include <memory>

Extractor::Extractor() = default;
Extractor::Extractor(Extractor &&) = default;

Extractor::~Extractor()
{
    qDeleteAll(m_rules);
}

bool Extractor::load(const QString& fileName)
{
    qCDebug(SEMANTIC_LOG) << "loading" << fileName;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() != QXmlStreamReader::StartElement)
            continue;
        if (reader.name() == QLatin1String("extractor"))
            continue;

        if (reader.name() == QLatin1String("filter")) {
            ExtractorFilter f;
            if (!f.load(reader))
                return false;
            m_filters.push_back(std::move(f));
            continue;
        }

        std::unique_ptr<ExtractorRule> rule;
        if (reader.name() == QLatin1String("variable"))
            rule.reset(new ExtractorVariableRule);
        else if (reader.name() == QLatin1String("class"))
            rule.reset(new ExtractorClassRule);
        else if (reader.name() == QLatin1String("property"))
            rule.reset(new ExtractorPropertyRule);
        else
            return false;
        if (!rule->load(reader))
            return false;
        m_rules.push_back(rule.release());
    }

    qCDebug(SEMANTIC_LOG) << fileName << "loaded!";
    return true;
}

QVector<ExtractorRule*> Extractor::rules() const
{
    return m_rules;
}

const std::vector<ExtractorFilter>& Extractor::filters() const
{
    return m_filters;
}
