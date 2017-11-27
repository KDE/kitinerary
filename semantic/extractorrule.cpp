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

#include "extractorrule.h"
#include "extractorcontext.h"
#include "extractorengine.h"
#include "semantic_debug.h"

#include <QDateTime>
#include <QXmlStreamReader>

#include <memory>

ExtractorRule::~ExtractorRule() = default;

QString ExtractorRule::name() const
{
    return m_name;
}

QString ExtractorRule::type() const
{
    return m_type;
}

bool ExtractorRule::repeats() const
{
    return m_repeat;
}

QString ExtractorRule::value(const QRegularExpressionMatch &match, ExtractorContext *context) const
{
    auto v = m_value;
    while (true) {
        const auto begin = v.indexOf(QLatin1String("${"));
        if (begin < 0) {
            break;
        }
        const auto end = v.indexOf(QLatin1Char('}'), begin + 3);
        const auto varName = v.mid(begin + 2, end - begin - 2);
        bool isNum = false;
        const auto captureIdx = varName.toInt(&isNum);
        if (isNum) {
            v.replace(begin, end - begin + 1, match.captured(captureIdx));
        } else {
            v.replace(begin, end - begin + 1, context->variableValue(varName));
        }
    }

    return v.trimmed();
}

QString ExtractorRule::format() const
{
    return m_format;
}

bool ExtractorRule::load(QXmlStreamReader &reader)
{
    m_name = reader.attributes().value(QLatin1String("name")).toString();
    m_type = reader.attributes().value(QLatin1String("type")).toString();
    m_value = reader.attributes().value(QLatin1String("value")).toString();
    m_format = reader.attributes().value(QLatin1String("format")).toString();
    m_repeat = reader.attributes().value(QLatin1String("repeat")) == QLatin1String("true");
    m_regexp.setPattern(reader.attributes().value(QLatin1String("match")).toString());
    if (!m_regexp.isValid()) {
        qCWarning(SEMANTIC_LOG) << m_regexp.errorString() << m_regexp.pattern() << "at offset" << m_regexp.patternErrorOffset();
    }
    return true;
}

bool ExtractorVariableRule::match(ExtractorContext *context) const
{
    const auto res = m_regexp.match(context->engine()->text(), context->offset());
    if (res.hasMatch()) {
        qCDebug(SEMANTIC_LOG) << name() << res.captured() << context->offset() << res.capturedEnd() << context->engine()->text().mid(context->offset(), 20);
        context->setVariable(name(), value(res, context));
        context->setOffset(res.capturedEnd());
    }
    return res.hasMatch();
}

ExtractorClassRule::~ExtractorClassRule()
{
    qDeleteAll(m_rules);
}

bool ExtractorClassRule::load(QXmlStreamReader &reader)
{
    if (!ExtractorRule::load(reader)) {
        return false;
    }

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::EndElement) {
            return true;
        }
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        std::unique_ptr<ExtractorRule> rule;
        if (reader.name() == QLatin1String("variable")) {
            rule.reset(new ExtractorVariableRule);
            if (!rule->load(reader)) {
                return false;
            }
            reader.skipCurrentElement();
        } else if (reader.name() == QLatin1String("class")) {
            rule.reset(new ExtractorClassRule);
            if (!rule->load(reader)) {
                return false;
            }
        } else if (reader.name() == QLatin1String("property")) {
            rule.reset(new ExtractorPropertyRule);
            if (!rule->load(reader)) {
                return false;
            }
            reader.skipCurrentElement();
        } else {
            return false;
        }

        m_rules.push_back(rule.release());
    }

    return true;
}

bool ExtractorClassRule::match(ExtractorContext *context) const
{
    const auto res = m_regexp.match(context->engine()->text(), context->offset());
    if (res.hasMatch()) {
        context->setOffset(res.capturedEnd());
    }
    return res.hasMatch();
}

QVector<ExtractorRule *> ExtractorClassRule::rules() const
{
    return m_rules;
}

bool ExtractorPropertyRule::match(ExtractorContext *context) const
{
    const auto res = m_regexp.match(context->engine()->text(), context->offset());
    if (res.hasMatch()) {
        auto val = value(res, context);
        if (type() == QLatin1String("dateTime") && !format().isEmpty()) {
            const auto dt = QDateTime::fromString(val, format());
            val = dt.toString(Qt::ISODate);
        }

        context->setProperty(name(), val);
        context->setOffset(res.capturedEnd());
    }
    return res.hasMatch();
}
