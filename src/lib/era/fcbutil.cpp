/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbutil.h"

using namespace KItinerary;

QString FcbUtil::stringifyUicStationIdentifier(int num, const QByteArray &ia5)
{
    if (num >= 10'00000 && num <= 99'99999) {
      return QLatin1StringView("uic:") + QString::number(num);
    }
    if (ia5.size() == 7) {
      return QLatin1StringView("uic:") + QString::fromLatin1(ia5);
    }

    return {};
}

QString FcbUtil::stringifyStationIdentifier(bool numIsSet, int num, const QByteArray &ia5)
{
    if (numIsSet) {
        return QString::number(num);
    }
    return QString::fromLatin1(ia5);
}

QString FcbUtil::classCodeToString(Fcb::TravelClassType classCode)
{
    switch (classCode) {
        case Fcb::notApplicable: return {};
        case Fcb::first: return QString::number(1);
        case Fcb::second: return QString::number(2);
        default:
            qCWarning(Log) << "Unhandled FCB class code" << classCode;
    }
    return {};
}

QDate FcbUtil::decodeDate(int year, std::optional<int> day)
{
    QDate d(year, 1, 1);
    if (day) {
        d = d.addDays((*day) - 1);
    }
    return d;
}

QDate FcbUtil::decodeDifferentialDate(const QDate &base, int year, std::optional<int> day)
{
    QDate d(base.year(), 1, 1);
    d = d.addYears(year);
    if (day) {
        d = d.addDays(*day);
    }
    return d;
}

QDateTime FcbUtil::issuingDateTime(int year, int day, std::optional<int> time)
{
    QDate date = decodeDate(year, day);
    if (time) {
        return QDateTime(date, QTime(0,0).addSecs(*time * 60), QTimeZone::UTC);
    }
    return QDateTime(date, {});
}

QDateTime FcbUtil::decodeDifferentialTime(const QDateTime &baseDt, int day, std::optional<int> time, std::optional<int> utcOffset)
{
    if (!time) {
        return {};
    }

    QDate d = baseDt.date().addDays(day);
    QTime t = QTime(0, 0).addSecs((*time) * 60);
    if (utcOffset) {
        return QDateTime(d, t, QTimeZone::fromSecondsAheadOfUtc(- (*utcOffset) * 15 * 60));
    }
    return QDateTime(d, t);
}

QDateTime FcbUtil::decodeDifferentialStartTime(const QDateTime &baseDt, int day, std::optional<int> time, std::optional<int> utcOffset)
{
    QDate d = baseDt.date().addDays(day);
    QTime t = time ? QTime(0, 0).addSecs((*time) * 60) : QTime();
    if (utcOffset) {
        return QDateTime(d, t, QTimeZone::fromSecondsAheadOfUtc(- (*utcOffset) * 15 * 60));
    }
    return QDateTime(d, t);
}

QDateTime FcbUtil::decodeDifferentialEndTime(const QDateTime &baseDt, int day, std::optional<int> time, std::optional<int> utcOffset)
{
    QDate d = baseDt.date().addDays(day);
    QTime t = time ? QTime(0, 0).addSecs((*time) * 60) : QTime(23, 59, 59);
    if (utcOffset) {
        return QDateTime(d, t, QTimeZone::fromSecondsAheadOfUtc(- (*utcOffset) * 15 * 60));
    }
    return QDateTime(d, t);
}
