/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/rsp/rsp6decoder.cpp"

#include <KItinerary/ExtractorEngine>

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class Rsp6DecoderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // work around to get the qrc data with the RSP-6 keys loaded,
        // which would be omitted when linking with --as-needed here
        ExtractorEngine engine;
    }

    void testDecode()
    {
        const QByteArray input("06DNQL4XHVK00TTRCGPUQWNTHPGHWBPOUTKRWXAJKGHFBAPBCTOGUZQVTZTKKDEBQXPGRWZJRJBXJZPOHNJGIPDJWEGYWJXLVPGEEZBCUUELIJMOINPRZMSDQCZJGLIZLUTQHXMTPKWCMJISUXQLORAOVYXSOLGXXGMVUDXTMHAYMBLUTKPUPFCRNNTDBBDLNWSBPDUXYKSIMJSBYBURSCPUMFBZPEUTECHTIOXAH");

        const auto output = Rsp6Decoder::decode(input);
        QVERIFY(!output.isEmpty());
        QCOMPARE(output, QByteArray::fromHex("0092ec6c538a36ad00c30c39c99a6c24826ca0828c2ec800c07d128a00000000000000000000000000000000000000018251466a000a530b800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007737378f24651de7"));
    }
};

QTEST_GUILESS_MAIN(Rsp6DecoderTest)

#include "rsp6decodertest.moc"
