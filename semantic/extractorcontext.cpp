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

#include "extractorcontext.h"
#include "semantic_debug.h"

ExtractorContext::ExtractorContext(ExtractorEngine *engine, ExtractorContext* parent)
    : m_engine(engine)
    , m_parent(parent)
{
}

ExtractorContext::~ExtractorContext() = default;

QVector<ExtractorRule*>& ExtractorContext::rules()
{
    return m_rules;
}

int ExtractorContext::offset() const
{
    return m_offset;
}

void ExtractorContext::setOffset(int offset)
{
    m_offset = offset;
}

ExtractorEngine* ExtractorContext::engine() const
{
    return m_engine;
}

void ExtractorContext::setRules(const QVector<ExtractorRule*> &rules)
{
    m_rules = rules;
}

QString ExtractorContext::variableValue(const QString& name) const
{
    const auto it = m_variables.constFind(name);
    if (it != m_variables.constEnd())
        return it.value();
    if (m_parent)
        return m_parent->variableValue(name);
    return {};
}

void ExtractorContext::setVariable(const QString& name, const QString& value)
{
    m_variables.insert(name, value);
    qCDebug(SEMANTIC_LOG) << m_variables;
}

void ExtractorContext::setProperty(const QString& name, const QJsonValue& value)
{
    m_obj.insert(name, value);
}

QJsonObject ExtractorContext::object() const
{
    return m_obj;
}
