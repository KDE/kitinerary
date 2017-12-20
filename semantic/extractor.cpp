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
#include "semantic_debug.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <memory>

Extractor::Extractor() = default;
Extractor::Extractor(Extractor &&) = default;
Extractor::~Extractor() = default;

bool Extractor::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (doc.isNull()) {
        qCWarning(SEMANTIC_LOG) << "Extractor loading error:" << fileName << error.errorString();
        return false;
    }

    const auto obj = doc.object();
    for (const auto &filterValue : obj.value(QLatin1String("filter")).toArray()) {
        ExtractorFilter f;
        if (!f.load(filterValue.toObject()))
            return false;
        m_filters.push_back(std::move(f));
    }

    const auto scriptName = obj.value(QLatin1String("script")).toString();
    QFileInfo fi(fileName);
    m_scriptName = fi.absolutePath() + QLatin1Char('/') + scriptName;
    return !m_filters.empty() && !m_scriptName.isEmpty() && QFile::exists(m_scriptName);
}

QString Extractor::scriptFileName() const
{
    return m_scriptName;
}

const std::vector<ExtractorFilter> &Extractor::filters() const
{
    return m_filters;
}
