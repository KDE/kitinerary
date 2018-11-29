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

#include "calendarhandler.h"
#include "extractorpostprocessor.h"
#include "jsonlddocument.h"

#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KCalCore;
using namespace KItinerary;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class CalendarHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateEvent_data()
    {
        QTest::addColumn<QString>("jsonFile");
        QTest::addColumn<QString>("icalFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/calendarhandlerdata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.json")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 4) + QStringLiteral("ics");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testCreateEvent()
    {
        QFETCH(QString, jsonFile);
        QFETCH(QString, icalFile);

        QFile f(jsonFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto inArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!inArray.isEmpty());
        const auto preData = JsonLdDocument::fromJson(inArray);
        QCOMPARE(inArray.size(), preData.size());

        ExtractorPostprocessor postproc;
        postproc.process(preData);
        QCOMPARE(inArray.size(), postproc.result().size());

        MemoryCalendar::Ptr refCal(new MemoryCalendar(QTimeZone{}));
        ICalFormat format;
        format.load(refCal, icalFile);

        const auto refEvents = refCal->rawEvents(KCalCore::EventSortStartDate, KCalCore::SortDirectionAscending);
        QCOMPARE(refEvents.size(), inArray.size());
        for (int i = 0; i < inArray.size(); ++i) {
            Event::Ptr newEvent(new Event);
            CalendarHandler::fillEvent(postproc.result(), newEvent);

            // sync volatile fields, we only care for differences elsewhere
            const auto &refEvent = refEvents.at(i);
            newEvent->setUid(refEvent->uid());
            newEvent->setLastModified(refEvent->lastModified());
            newEvent->setCreated(refEvent->created());

            if (*newEvent != *refEvent) {
                qDebug().noquote() << "Actual: " << format.toICalString(newEvent);
                qDebug().noquote() << "Expected: " << format.toICalString(refEvent);
            }

            QCOMPARE(newEvent->dtStart(), refEvent->dtStart());
            QCOMPARE(newEvent->dtEnd(), refEvent->dtEnd());
            QVERIFY(*newEvent == *refEvent);
        }
    }

    void testFindEvent_data()
    {
        QTest::addColumn<QString>("jsonFile");
        QTest::addColumn<QString>("icalFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/calendarhandlerdata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.json")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 4) + QStringLiteral("ics");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testFindEvent()
    {
        QFETCH(QString, jsonFile);
        QFETCH(QString, icalFile);

        QFile f(jsonFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto inArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!inArray.isEmpty());
        const auto preData = JsonLdDocument::fromJson(inArray);
        QCOMPARE(inArray.size(), preData.size());

        ExtractorPostprocessor postproc;
        postproc.process(preData);
        QCOMPARE(inArray.size(), postproc.result().size());

        MemoryCalendar::Ptr refCal(new MemoryCalendar(QTimeZone{}));
        ICalFormat format;
        format.load(refCal, icalFile);

        const auto event = CalendarHandler::findEvent(refCal, postproc.result().at(0));
        QVERIFY(event);
    }
};

QTEST_APPLESS_MAIN(CalendarHandlerTest)

#include "calendarhandlertest.moc"
