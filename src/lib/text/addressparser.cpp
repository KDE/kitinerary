/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressparser_p.h"

#include <KContacts/AddressFormat>
#include <KCountry>

#include <QDebug>
#include <QRegularExpression>

using namespace Qt::Literals;
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
    if (m_address.addressCountry().isEmpty() && !m_address.addressLocality().isEmpty() && m_address.postalCode().isEmpty()) {
        identifyFromPostalCode();
    }

    if ((m_address.postalCode().isEmpty() && !m_address.addressLocality().isEmpty()) ||
        (!m_address.postalCode().isEmpty() && m_address.addressLocality().contains(m_address.postalCode())))
    {
        splitPostalCode();
    }

    // crude fallback if the above doesn't work (yet), e.g. in the UK
    if (!m_address.postalCode().isEmpty() && m_address.addressLocality().contains(m_address.postalCode())) {
        m_address.setAddressLocality(m_address.addressLocality().remove(m_address.postalCode()).trimmed());
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
  return QLatin1StringView("?<") + captureName(field) + QLatin1Char('>');
}

static std::vector<KContacts::AddressFormatElement> postalCodeLine(const KContacts::AddressFormat &format)
{
    using namespace KContacts;
    auto startIt = format.elements().begin();
    auto endIt = startIt;
    enum : uint8_t {
        None = 0,
        HasLocality = 1,
        HasPostalCode = 2,
        HasBoth = 3,
    };
    uint8_t inRelevantLine = None;
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
    return std::vector<AddressFormatElement>(startIt, endIt);
}

void AddressParser::identifyFromPostalCode()
{
    KCountry matchedCountry;
    for (const auto &country : KCountry::allCountries()) {
        // TODO check script
        const auto format = KContacts::AddressFormatRepository::formatForCountry(country.alpha2(), KContacts::AddressFormatScriptPreference::Local);
        if (format.elements().empty() || format.postalCodeRegularExpression().isEmpty()) {
            continue;
        }

        const auto line = postalCodeLine(format);
        if (line.empty()) {
            continue;
        }

        // build regex for that format line
        QString regex('^'_L1);
        for (const auto &elem : line) {
            if (elem.isField()) {
                regex += (elem.field() == KContacts::AddressFormatField::PostalCode ? format.postalCodeRegularExpression() : "\\S.*"_L1);
            }
            else if (elem.isLiteral()) {
                regex += elem.literal();
            }
        }
        regex += '$'_L1;

        QRegularExpression re(regex);
        if (!re.isValid()) {
            qWarning() << "generated invalid address parsing pattern:" << regex;
            return;
        }

        // match against the input
        if (const auto match = re.match(m_address.addressLocality()); !match.hasMatch()) {
            continue;
        }

        if (matchedCountry.isValid()) {
            qDebug() << "ambiguous postal code, can't identify country based on that" << matchedCountry.alpha2() << country.alpha2();
            return;
        }
        matchedCountry = country;
    }

    if (matchedCountry.isValid()) {
        m_address.setAddressCountry(matchedCountry.alpha2());
    }
}

void AddressParser::splitPostalCode()
{
    const auto format = addressFormat();
    if (format.elements().empty() || format.postalCodeRegularExpression().isEmpty()) {
        return;
    }

    // find the format line containing the postal code and locality
    using namespace KContacts;
    const auto line = postalCodeLine(format);
    // TODO also handle the case the region is part of the same line
    if (line.empty() || std::count_if(line.begin(), line.end(), std::mem_fn(&AddressFormatElement::isField)) > 2) {
        return;
    }

    // build regex for that format line
    QString regex;
    regex.push_back(QLatin1Char('^'));
    for (auto it = line.begin(); it != line.end(); ++it) {
        if ((*it).isField()) {
          regex += QLatin1Char('(') + captureExpression((*it).field()) +
                   ((*it).field() == AddressFormatField::PostalCode
                        ? format.postalCodeRegularExpression()
                        : QLatin1StringView("\\S.*")) +
                   QLatin1Char(')');
        }
        if ((*it).isLiteral()) {
            // allow the first literal to be missing, those are typically country-prefixes those postal codes
            if (regex.size() == 1) {
                regex += "(:?"_L1 + (*it).literal() + ")?"_L1;
            } else {
                regex += (*it).literal();
            }
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
