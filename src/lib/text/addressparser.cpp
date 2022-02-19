/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressparser_p.h"

#include <KContacts/AddressFormat>

#include <QDebug>
#include <QRegularExpression>

using namespace KItinerary;

AddressParser::AddressParser() = default;
AddressParser::~AddressParser() = default;

void AddressParser::setFallbackCountry(const QString &countryCode)
{
    m_fallbackCountry = countryCode;
}

void AddressParser::parse(PostalAddress addr)
{
    m_address = addr;
    if (m_address.postalCode().isEmpty() && !m_address.addressLocality().isEmpty()) {
        splitPostalCode();
    }
}

PostalAddress AddressParser::result() const
{
    return m_address;
}

KContacts::AddressFormat AddressParser::addressFormat() const
{
    // TODO detect script
    return KContacts::AddressFormatRepository::formatForCountry(m_address.addressCountry().isEmpty() ? m_fallbackCountry : m_address.addressCountry(), KContacts::AddressFormatScriptPreference::Local);
}

static QString captureName(KContacts::AddressFormatField field)
{
    switch (field) {
        case KContacts::AddressFormatField::PostalCode:
            return QStringLiteral("postalCode");
        case KContacts::AddressFormatField::Locality:
            return QStringLiteral("locality");
        default:
            return {};
    }
}

static QString captureExpression(KContacts::AddressFormatField field)
{
    return QLatin1String("?<") + captureName(field) + QLatin1Char('>');
}

void AddressParser::splitPostalCode()
{
    const auto format = addressFormat();
    if (format.elements().empty() || format.postalCodeRegularExpression().isEmpty()) {
        return;
    }

    // find the format line containing the postal code and locality
    using namespace KContacts;
    auto startIt = format.elements().begin();
    auto endIt = startIt;
    enum {
        None = 0,
        HasLocality = 1,
        HasPostalCode = 2,
        HasBoth = 3,
    };
    int inRelevantLine = None;
    for (auto it = format.elements().begin(); it != format.elements().end(); ++it) {
        if ((*it).isSeparator() && inRelevantLine != HasBoth) {
            startIt = endIt = it;
            inRelevantLine = None;
        }
        if ((*it).isSeparator() && inRelevantLine == HasBoth) {
            endIt = it;
            inRelevantLine = None;
            break;
        }
        if ((*it).isField() && (*it).field() == AddressFormatField::Locality) {
            inRelevantLine |= HasLocality;
        }
        if ((*it).isField() && (*it).field() == AddressFormatField::PostalCode) {
            inRelevantLine |= HasPostalCode;
        }
    }
    if (inRelevantLine == HasBoth) {
        endIt = format.elements().end();
    }
    std::vector<AddressFormatElement> line(startIt, endIt);
    // TODO also handle the case the region is part of the same line
    if (line.empty() || std::count_if(line.begin(), line.end(), std::mem_fn(&AddressFormatElement::isField)) > 2) {
        return;
    }

    // build regex for that format line
    QString regex;
    regex.push_back(QLatin1Char('^'));
    for (auto it = line.begin(); it != line.end(); ++it) {
        if ((*it).isField()) {
            regex += QLatin1Char('(') + captureExpression((*it).field())
                  + ((*it).field() == AddressFormatField::PostalCode ? format.postalCodeRegularExpression() : QLatin1String("\\S.*"))
                  + QLatin1Char(')');
        }
        if ((*it).isLiteral()) {
            regex += (*it).literal();
        }
    }

    QRegularExpression re(regex);
    if (!re.isValid()) {
        qWarning() << "generated invalid address parsing pattern:" << regex;
        return;
    }

    // match against the input
    const auto match = re.match(m_address.addressLocality());
    if (!match.hasMatch()) {
        return;
    }

    const auto postalCode = match.captured(captureName(AddressFormatField::PostalCode));
    const auto locality = match.captured(captureName(AddressFormatField::Locality));
    if (!locality.isEmpty() && !postalCode.isEmpty()) {
        m_address.setPostalCode(postalCode);
        m_address.setAddressLocality(locality);
    }
}
