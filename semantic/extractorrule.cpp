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

ExtractorRule::ExtractorRule(ExtractorRule::Type type)
    : m_ruleType(type)
{
}

ExtractorRule::~ExtractorRule()
{
    qDeleteAll(m_rules);
}

ExtractorRule::Type ExtractorRule::ruleType() const
{
    return m_ruleType;
}

bool ExtractorRule::hasSubRules() const
{
    return !m_rules.empty();
}

QVector<ExtractorRule*> ExtractorRule::rules() const
{
    return m_rules;
}

QString ExtractorRule::name() const
{
    return m_name;
}

QString ExtractorRule::dataType() const
{
    return m_dataType;
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

QLocale ExtractorRule::locale() const
{
    return m_locale;
}

bool ExtractorRule::load(QXmlStreamReader &reader)
{
    m_name = reader.attributes().value(QLatin1String("name")).toString();
    m_dataType = reader.attributes().value(QLatin1String("type")).toString();
    m_value = reader.attributes().value(QLatin1String("value")).toString();
    m_format = reader.attributes().value(QLatin1String("format")).toString();
    m_repeat = reader.attributes().value(QLatin1String("repeat")) == QLatin1String("true");
    m_regexp.setPattern(reader.attributes().value(QLatin1String("match")).toString());
    if (!m_regexp.isValid()) {
        qCWarning(SEMANTIC_LOG) << m_regexp.errorString() << m_regexp.pattern() << "at offset" << m_regexp.patternErrorOffset();
    }
    if (reader.attributes().hasAttribute(QLatin1String("locale"))) {
        m_locale = QLocale(reader.attributes().value(QLatin1String("locale")).toString());
    }

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::EndElement) {
            return true;
        }
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        auto rule = fromXml(reader);
        if (!rule) {
            return false;
        }
        m_rules.push_back(rule);
    }

    return false;
}

ExtractorRule *ExtractorRule::fromXml(QXmlStreamReader &reader)
{
    std::unique_ptr<ExtractorRule> rule;
    QStringRef readerName = reader.name();
    if (readerName == QLatin1String("variable")) {
        rule.reset(new ExtractorVariableRule);
    } else if (readerName == QLatin1String("class")) {
        rule.reset(new ExtractorClassRule);
    } else if (readerName == QLatin1String("property")) {
        rule.reset(new ExtractorPropertyRule);
    } else if (readerName == QLatin1String("break")) {
        rule.reset(new ExtractorBreakRule);
    } else {
        return nullptr;
    }
    if (!rule->load(reader)) {
        return nullptr;
    }
    return rule.release();
}

bool ExtractorRule::match(ExtractorContext *context) const
{
    // use QString::midRef(offset) rather than match(text(), offset) as that makes '^' matches work
    const auto res = m_regexp.match(context->engine()->text().midRef(context->offset()));
    if (res.hasMatch()) {
        qCDebug(SEMANTIC_LOG) << name() << res.captured() << context->offset() << res.capturedEnd() << context->engine()->text().midRef(context->offset(), 20);
        processMatch(res, context);
        context->setOffset(res.capturedEnd() + context->offset());
    }
    return res.hasMatch();
}

void ExtractorRule::processMatch(const QRegularExpressionMatch &match, ExtractorContext *context) const
{
    Q_UNUSED(match);
    Q_UNUSED(context);
}

ExtractorVariableRule::ExtractorVariableRule()
    : ExtractorRule(ExtractorRule::Variable)
{
}

void ExtractorVariableRule::processMatch(const QRegularExpressionMatch &match, ExtractorContext *context) const
{
    context->setVariable(name(), value(match, context));
}

ExtractorClassRule::ExtractorClassRule()
    : ExtractorRule(ExtractorRule::Class)
{
}

ExtractorPropertyRule::ExtractorPropertyRule()
    : ExtractorRule(ExtractorRule::Property)
{
}

void ExtractorPropertyRule::processMatch(const QRegularExpressionMatch &match, ExtractorContext *context) const
{
    auto val = value(match, context);
    if (dataType() == QLatin1String("dateTime") && !format().isEmpty()) {
        const auto dt = locale().toDateTime(val, format());
        val = dt.toString(Qt::ISODate);
    }

    context->setProperty(name(), val);
}

ExtractorBreakRule::ExtractorBreakRule()
    : ExtractorRule(ExtractorRule::Break)
{
}
