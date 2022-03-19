/*
  SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarhandler.h"
#include "extractorpostprocessor.h"
#include "jsonlddocument.h"

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QObject>
#include <QTest>

using namespace KCalendarCore;
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
        const auto postData = postproc.result();
        QVERIFY(std::all_of(postData.begin(), postData.end(), CalendarHandler::canCreateEvent));
        QCOMPARE(inArray.size(), postData.size());

        MemoryCalendar::Ptr refCal(new MemoryCalendar(QTimeZone::systemTimeZone()));
        ICalFormat format;
        format.load(refCal, icalFile);

        const auto refEvents = refCal->rawEvents(KCalendarCore::EventSortStartDate, KCalendarCore::SortDirectionAscending);
        QCOMPARE(refEvents.size(), 1);
        Event::Ptr newEvent(new Event);
        CalendarHandler::fillEvent(postData, newEvent);

        // sync volatile fields, we only care for differences elsewhere
        const auto &refEvent = refEvents.at(0);
        newEvent->setUid(refEvent->uid());
        newEvent->setLastModified(refEvent->lastModified());
        newEvent->setCreated(refEvent->created());

        if (*newEvent != *refEvent) {
            qDebug().noquote() << "Actual: " << format.toICalString(newEvent).remove(QLatin1Char('\r'));
            qDebug().noquote() << "Expected: " << format.toICalString(refEvent).remove(QLatin1Char('\r'));

            QFile failFile(icalFile + QLatin1String(".fail"));
            QVERIFY(failFile.open(QFile::WriteOnly));
            failFile.write(format.toICalString(newEvent).remove(QLatin1Char('\r')).toUtf8());
            failFile.close();

            QProcess proc;
            proc.setProcessChannelMode(QProcess::ForwardedChannels);
            proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), icalFile, failFile.fileName()});
            QVERIFY(proc.waitForFinished());
        }

        QCOMPARE(newEvent->dtStart(), refEvent->dtStart());
        QCOMPARE(newEvent->dtEnd(), refEvent->dtEnd());
        QCOMPARE(newEvent->customProperty("KITINERARY", "RESERVATION"), refEvent->customProperty("KITINERARY", "RESERVATION"));
        QCOMPARE(newEvent->description(), refEvent->description());
        QVERIFY(*newEvent == *refEvent);
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

        MemoryCalendar::Ptr refCal(new MemoryCalendar(QTimeZone::systemTimeZone()));
        ICalFormat format;
        format.load(refCal, icalFile);

        const auto events = CalendarHandler::findEvents(refCal, postproc.result().at(0));
        QCOMPARE(events.size(), 1);
        QVERIFY(events[0]);
    }

    void testFindEventForCancellation()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/mergedata/cancellation.rhs.json"));
        QVERIFY(f.open(QFile::ReadOnly));
        const auto in = JsonLdDocument::fromJson(QJsonDocument::fromJson(f.readAll()).array());
        QCOMPARE(in.size(), 1);
        const auto cancel = in[0];

        MemoryCalendar::Ptr refCal(new MemoryCalendar(QTimeZone::systemTimeZone()));
        ICalFormat format;
        format.load(refCal, QStringLiteral(SOURCE_DIR "/calendarhandlerdata/to-be-cancelled.ics"));

        const auto events = CalendarHandler::findEvents(refCal, cancel);
        QCOMPARE(events.size(), 2);
        QVERIFY(events[0]);
        QVERIFY(events[1]);
    }
};

QTEST_APPLESS_MAIN(CalendarHandlerTest)

#include "calendarhandlertest.moc"
