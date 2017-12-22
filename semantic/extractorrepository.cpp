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

#include "extractorrepository.h"
#include "extractor.h"
#include "semantic_debug.h"

#include <KMime/Content>

#include <QDirIterator>
#include <QStandardPaths>

ExtractorRepository::ExtractorRepository()
{
    loadExtractors();
}

ExtractorRepository::~ExtractorRepository() = default;

std::vector<const Extractor *> ExtractorRepository::extractorsForMessage(KMime::Content *part) const
{
    std::vector<const Extractor *> v;
    if (!part) {
        return v;
    }

    for (auto it = m_extractors.begin(), end = m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            auto header = part->headerByType(filter.headerName());
            if (!header && part->topLevel()) {
                header = part->topLevel()->headerByType(filter.headerName());
            }
            if (!header) {
                continue;
            }
            const auto headerData = header->asUnicodeString();
            if (filter.matches(headerData)) {
                v.push_back(&(*it));
                break;
            }
        }
    }

    return v;
}

void ExtractorRepository::loadExtractors()
{
    QStringList searchDirs = { QStringLiteral(":/org.kde.pim") };
    searchDirs += QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (const auto &dir : qAsConst(searchDirs)) {
        QDirIterator it(dir + QStringLiteral("/messageviewer/semantic/extractors"), {QStringLiteral("*.json")}, QDir::Files);
        while (it.hasNext()) {
            Extractor e;
            if (e.load(it.next())) {
                m_extractors.push_back(std::move(e));
            }
        }
    }
}
