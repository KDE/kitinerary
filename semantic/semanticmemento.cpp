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

#include "semanticmemento.h"

#include <KMime/ContentIndex>

SemanticMemento::~SemanticMemento() = default;

void SemanticMemento::detach()
{
}

bool SemanticMemento::isEmpty() const
{
    return m_data.isEmpty();
}

bool SemanticMemento::isParsed(const KMime::ContentIndex &index) const
{
    return m_parsedParts.contains(index);
}

void SemanticMemento::setParsed(const KMime::ContentIndex &index)
{
    m_parsedParts.insert(index);
}

QVector<QVariant> SemanticMemento::data() const
{
    return m_data;
}

void SemanticMemento::setData(const QVector<QVariant> &data)
{
    m_data = data;
}

bool SemanticMemento::hasStructuredData() const
{
    return m_foundStructuredData && !isEmpty();
}

void SemanticMemento::setStructuredDataFound(bool f)
{
    m_foundStructuredData = f;
}
