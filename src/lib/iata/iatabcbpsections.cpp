/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbpsections.h"
#include "iatabcbpconstants_p.h"
#include "logging.h"

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;
using namespace KItinerary::IataBcbpConstants;

QString IataBcbpSectionBase::readString(int offset, int length) const
{
    if (m_data.size() >= offset + length) {
        return m_data.mid(offset, length).trimmed().toString();
    }
    return {};
}

int IataBcbpSectionBase::readNumericValue(int offset, int length, int base) const
{
    if (m_data.size() >= offset + length) {
        return m_data.mid(offset, length).toInt(nullptr, base);
    }
    return 0;
}


IataBcbpUniqueMandatorySection::IataBcbpUniqueMandatorySection(QStringView data)
{
    m_data = data;
}

bool IataBcbpUniqueMandatorySection::isValid() const
{
    const auto legCount = numberOfLegs();
    return legCount >= 1 && legCount <= 4;
}

IataBcbpUniqueConditionalSection::IataBcbpUniqueConditionalSection(QStringView data)
{
    if (data.size() < MinimumUniqueConditionalSize) {
        return;
    }
    m_data = data;
    m_data = data.left(MinimumUniqueConditionalSize + fieldSize());
}

bool IataBcbpUniqueConditionalSection::isValid() const
{
    if (m_data.size() >= 11) {
        // issue date
        if (std::any_of(m_data.begin() + 8, m_data.begin() + 11, [](auto c) { return !c.isDigit() && c != ' '_L1; }) || dayOfIssue() > 366) {
            return false;
        }
    }
    return true;
}

QDate IataBcbpUniqueConditionalSection::dateOfIssue(const QDateTime &contextDate) const
{
    const auto day = dayOfIssue() - 1;
    if (m_data.size() < 11 || day < 0) {
        return {};
    }

    const auto year = contextDate.date().year() - contextDate.date().year() % 10 + yearOfIssue();
    const auto d = QDate(year, 1, 1).addDays(day);
    // TODO shouldn't this rather be d > contextDate?
    if (year > contextDate.date().year()) {
        return QDate(year - 10, 1, 1).addDays(day);
    }
    return d;
}

IataBcbpRepeatedMandatorySection::IataBcbpRepeatedMandatorySection(QStringView data)
{
    m_data = data;
}

static bool isValidAirportCode(QStringView s)
{
    return std::all_of(s.begin(), s.end(), [](const QChar c) { return c.isLetter() && c.isUpper(); });
}

bool IataBcbpRepeatedMandatorySection::isValid() const
{
    if (m_data.size() < RepeatedMandatoryMinimalViableSize) {
        return false;
    }

    return isValidAirportCode(m_data.mid(7, 3))
        && isValidAirportCode(m_data.mid(10, 3))
        && std::all_of(m_data.begin() + 21, m_data.begin() + 24, [](auto c) { return c.isDigit() || c == ' '_L1; })
        && dayOfFlight() <= 366;
}

QDate IataBcbpRepeatedMandatorySection::dateOfFlight(const QDateTime& contextDate) const
{
    const auto day = dayOfFlight() - 1;
    if (day < 0) {
        return {}; // no set
    }
    const auto d = QDate(contextDate.date().year(), 1, 1).addDays(day);
    if (d < contextDate.date()) {
        return QDate(d.year() + 1, 1, 1).addDays(day);
    }
    return d;
}

IataBcbpRepeatedConditionalSection::IataBcbpRepeatedConditionalSection(QStringView data)
{
    if (data.size() < 2) {
        return;
    }
    m_data = data;
    m_data = data.left(conditionalFieldSize() + 2);
}

IataBcbpSecuritySection::IataBcbpSecuritySection(QStringView data)
{
    if (data.size() < MinimumSecuritySectionSize) {
        return;
    }
    m_data = data;
    m_data = data.left(size() + MinimumSecuritySectionSize);
}

#include "moc_iatabcbpsections.cpp"
