/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/ExtractorInput>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QVariant>

using namespace KItinerary;

class ExtractorInputTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTypeFromContent_data()
    {
        QTest::addColumn<QByteArray>("data");
        QTest::addColumn<ExtractorInput::Type>("type");

        QTest::newRow("empty") << QByteArray() << ExtractorInput::Unknown;
        QTest::newRow("html") << QByteArray("<html>") << ExtractorInput::Html;
        QTest::newRow("html padded") << QByteArray("      <!DOCTYPE html>") << ExtractorInput::Html;
        QTest::newRow("json") << QByteArray(R"({"@type": "Foo"})") << ExtractorInput::JsonLd;
        QTest::newRow("json padded") << QByteArray("          []") << ExtractorInput::JsonLd;
        QTest::newRow("pkpass") << QByteArray("PK\x03\x04") << ExtractorInput::PkPass;
        QTest::newRow("pdf") << QByteArray("%PDF") << ExtractorInput::Pdf;
        QTest::newRow("text") << QByteArray("abc def") << ExtractorInput::Unknown;
        QTest::newRow("ical") << QByteArray("BEGIN:VCALENDAR\nEND:VCALENDAR") << ExtractorInput::ICal;
        QTest::newRow("email") << QByteArray("From: null@kde.org\nTo: foo@localhost\n\n") << ExtractorInput::Email;
        QTest::newRow("mbox") << QByteArray("From null@kde.org Mon Jan 01 12:34:56 1970\n") << ExtractorInput::Email;
    }

    void testTypeFromContent()
    {
        QFETCH(QByteArray, data);
        QFETCH(ExtractorInput::Type, type);
        QCOMPARE(ExtractorInput::typeFromContent(data), type);
    }

    void testTypeFromMimeType_data()
    {
        QTest::addColumn<QString>("mimeType");
        QTest::addColumn<ExtractorInput::Type>("type");

        QTest::newRow("empty") << QString() << ExtractorInput::Unknown;
        QTest::newRow("html") << QStringLiteral("text/html") << ExtractorInput::Html;
        QTest::newRow("json") << QStringLiteral("application/json") << ExtractorInput::JsonLd;
        QTest::newRow("json+ld") << QStringLiteral("application/ld+json") << ExtractorInput::JsonLd;
        QTest::newRow("pkpass") << QStringLiteral("application/vnd.apple.pkpass") << ExtractorInput::PkPass;
        QTest::newRow("pdf") << QStringLiteral("application/pdf") << ExtractorInput::Pdf;
        QTest::newRow("text") << QStringLiteral("text/plain") << ExtractorInput::Text;
        QTest::newRow("ical") << QStringLiteral("text/calendar") << ExtractorInput::ICal;
        QTest::newRow("email") << QStringLiteral("message/rfc822") << ExtractorInput::Email;
    }

    void testTypeFromMimeType()
    {
        QFETCH(QString, mimeType);
        QFETCH(ExtractorInput::Type, type);
        QCOMPARE(ExtractorInput::typeFromMimeType(mimeType), type);
    }

    void testTypeFromFileName_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<ExtractorInput::Type>("type");

        QTest::newRow("empty") << QString() << ExtractorInput::Unknown;
        QTest::newRow("html") << QStringLiteral("foo.html") << ExtractorInput::Html;
        QTest::newRow("html 2") << QStringLiteral("FOO.HTM") << ExtractorInput::Html;
        QTest::newRow("json") << QStringLiteral("foo.json") << ExtractorInput::JsonLd;
        QTest::newRow("json+ld") << QStringLiteral("foo.JSONLD") << ExtractorInput::JsonLd;
        QTest::newRow("pkpass") << QStringLiteral("foo.pkpass") << ExtractorInput::PkPass;
        QTest::newRow("pdf") << QStringLiteral("foo.pdf") << ExtractorInput::Pdf;
        QTest::newRow("text") << QStringLiteral("foo.txt") << ExtractorInput::Text;
        QTest::newRow("ical") << QStringLiteral("foo.ics") << ExtractorInput::ICal;
        QTest::newRow("email") << QStringLiteral("foo.mbox") << ExtractorInput::Email;
    }

    void testTypeFromFileName()
    {
        QFETCH(QString, fileName);
        QFETCH(ExtractorInput::Type, type);
        QCOMPARE(ExtractorInput::typeFromFileName(fileName), type);
    }

    void testTypeEnumToString()
    {
        QCOMPARE(ExtractorInput::typeToString(ExtractorInput::Unknown), QString());
        QCOMPARE(ExtractorInput::typeToString(ExtractorInput::Pdf), QLatin1String("Pdf"));
    }

    void testTypeEnumFromString()
    {
        QCOMPARE(ExtractorInput::typeFromName(QStringLiteral("HTML")), ExtractorInput::Html);
        QCOMPARE(ExtractorInput::typeFromName(QStringLiteral("text")), ExtractorInput::Text);
        QCOMPARE(ExtractorInput::typeFromName(QStringLiteral("Pdf")), ExtractorInput::Pdf);
        QCOMPARE(ExtractorInput::typeFromName(QStringLiteral("something else")), ExtractorInput::Unknown);
        QCOMPARE(ExtractorInput::typeFromName(QString()), ExtractorInput::Unknown);
    }
};

QTEST_APPLESS_MAIN(ExtractorInputTest)

#include "extractorinputtest.moc"