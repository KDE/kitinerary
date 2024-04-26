/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatabcbp.h"
#include "iatabcbpconstants_p.h"
#include "logging.h"

#include <QScopeGuard>

#include <cctype>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;
using namespace KItinerary::IataBcbpConstants;

IataBcbp::IataBcbp() = default;

IataBcbp::IataBcbp(const QString& data)
{
    if (data.size() < MinimumViableSize || data[0] != 'M'_L1 || !data[1].isDigit()) {
        return;
    }
    const auto trimmed = QStringView(data).trimmed(); // tolerance against e.g. trailing newlines
    if (std::any_of(trimmed.begin(), trimmed.end(), [](QChar c) { return c.row() != 0 || !c.isPrint(); })) {
        return;
    }
    m_data = data;
    auto resetOnInvalid = qScopeGuard([this] { m_data.clear(); });

    if (!uniqueMandatorySection().isValid()) {
        return;
    }
    if (hasUniqueConditionalSection() && !uniqueConditionalSection().isValid()) {
        return;
    }

    const auto legCount = uniqueMandatorySection().numberOfLegs();
    int offset = UniqueMandatorySize;
    for (int i = 0; i < legCount; ++i) {
        if (offset > m_data.size()) {
            return;
        }
        auto rms = IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset));
        if (!rms.isValid()) {
            return;
        }
        offset += rms.variableFieldSize() + RepeatedMandatorySize;
    }

    resetOnInvalid.dismiss();
}

IataBcbp::~IataBcbp() = default;

bool IataBcbp::isValid() const
{
    return !m_data.isEmpty();
}

IataBcbpUniqueMandatorySection IataBcbp::uniqueMandatorySection() const
{
    return IataBcbpUniqueMandatorySection(QStringView(m_data).left(UniqueMandatorySize));
}

bool IataBcbp::hasUniqueConditionalSection() const
{
    return (m_data.size() > (UniqueMandatorySize + RepeatedMandatorySize))
        && (m_data.at(UniqueMandatorySize + RepeatedMandatorySize) == '>'_L1)
        && repeatedMandatorySection(0).variableFieldSize() > MinimumUniqueConditionalSize;
}

IataBcbpUniqueConditionalSection IataBcbp::uniqueConditionalSection() const
{
    if (hasUniqueConditionalSection()) {
        return IataBcbpUniqueConditionalSection(QStringView(m_data).mid(UniqueMandatorySize + RepeatedMandatorySize));
    }
    return IataBcbpUniqueConditionalSection(QStringView());
}

IataBcbpRepeatedMandatorySection IataBcbp::repeatedMandatorySection(int leg) const
{
    int offset = UniqueMandatorySize;
    for (auto i = 0; i < leg; ++i) {
        offset += RepeatedMandatorySize + IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset)).variableFieldSize();
    }
    return IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset, RepeatedMandatorySize));
}

IataBcbpRepeatedConditionalSection IataBcbp::repeatedConditionalSection(int leg) const
{
    int offset = UniqueMandatorySize;
    if (leg == 0 && hasUniqueConditionalSection()) {
        offset += uniqueConditionalSection().fieldSize() + MinimumUniqueConditionalSize;
    }
    for (auto i = 0; i < leg; ++i) {
        offset += RepeatedMandatorySize + IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset)).variableFieldSize();
    }
    return IataBcbpRepeatedConditionalSection(QStringView(m_data).mid(offset + RepeatedMandatorySize));
}

QString IataBcbp::airlineUseSection(int leg) const
{
    int offset = UniqueMandatorySize;
    for (auto i = 0; i < leg; ++i) {
        offset += RepeatedMandatorySize + IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset)).variableFieldSize();
    }
    auto length = IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset)).variableFieldSize();
    if (leg == 0 && hasUniqueConditionalSection()) {
        const auto s = uniqueConditionalSection().fieldSize() + MinimumUniqueConditionalSize;
        offset += uniqueConditionalSection().fieldSize() + MinimumUniqueConditionalSize;
        length -= s;
    }
    if (leg == 0 && !hasUniqueConditionalSection()) { // Easyjet special case that has a airline use section right after the mandatory block
        return m_data.mid(offset + RepeatedMandatorySize, length);
    }
    const auto offset2 = IataBcbpRepeatedConditionalSection(QStringView(m_data).mid(offset + RepeatedMandatorySize)).conditionalFieldSize() + 2 + RepeatedMandatorySize;
    return m_data.mid(offset + offset2, length - offset2 + RepeatedMandatorySize);
}

bool IataBcbp::hasSecuritySection() const
{
    int offset = UniqueMandatorySize;
    for (auto i = 0; i < uniqueMandatorySection().numberOfLegs(); ++i) {
        offset += RepeatedMandatorySize + IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset)).variableFieldSize();
    }
    return offset < m_data.size() && m_data[offset] == '^'_L1;
}

IataBcbpSecuritySection IataBcbp::securitySection() const
{
    int offset = UniqueMandatorySize;
    for (auto i = 0; i < uniqueMandatorySection().numberOfLegs(); ++i) {
        offset += RepeatedMandatorySize + IataBcbpRepeatedMandatorySection(QStringView(m_data).mid(offset)).variableFieldSize();
    }
    return IataBcbpSecuritySection(QStringView(m_data).mid(offset));
}

QString IataBcbp::rawData() const
{
    return m_data;
}

bool IataBcbp::maybeIataBcbp(const QByteArray &data)
{
    return data.size() >= MinimumViableSize && data[0] == 'M' && std::isdigit(data[1]);
}

bool IataBcbp::maybeIataBcbp(const QString &data)
{
    return data.size() >= MinimumViableSize && data[0] == 'M'_L1 && data[1].isDigit();
}

#include "moc_iatabcbp.cpp"
