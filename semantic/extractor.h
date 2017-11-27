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

#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "extractorfilter.h"

#include <QVector>
#include <vector>

class ExtractorRule;
class QString;

/** A single unstructured data extraction rule set. */
class Extractor
{
public:
    Extractor();
    Extractor(const Extractor &) = delete;
    Extractor(Extractor &&);
    ~Extractor();

    bool load(const QString &fileName);

    QVector<ExtractorRule *> rules() const;
    const std::vector<ExtractorFilter> &filters() const;

private:
    QVector<ExtractorRule *> m_rules;
    std::vector<ExtractorFilter> m_filters;
};

#endif // EXTRACTOR_H
