/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <vdvcertificate_p.h>

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>

#include <vector>

using namespace KItinerary;

static std::vector<QString> listCerts()
{
    QProcess proc;
    proc.setProgram(QStringLiteral("kioclient5"));
    proc.setArguments({QStringLiteral("ls"), QStringLiteral("ldap://ldap-vdv-ion.telesec.de:389/ou=VDV%20KA,o=VDV%20Kernapplikations%20GmbH,c=de")});
    proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    proc.start();
    if (!proc.waitForFinished() || proc.exitStatus() != QProcess::NormalExit) {
        qFatal("Failed to list certificates from LDAP server.");
    }

    std::vector<QString> certs;
    for (const auto &line : proc.readAllStandardOutput().split('\n')) {
        if (line.size() <= 5) {
            continue;
        }
        certs.push_back(QString::fromUtf8(line.left(line.size() - 5)));
    }
    return certs;
}

static void downloadCert(const QString &certName)
{
    QProcess proc;
    proc.setProgram(QStringLiteral("kioclient5"));
    proc.setArguments({QStringLiteral("cat"), QStringLiteral("ldap://ldap-vdv-ion.telesec.de:389/cn=") + certName + QStringLiteral(",ou=VDV%20KA,o=VDV%20Kernapplikations%20GmbH,c=de")});
    proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    proc.start();
    if (!proc.waitForFinished() || proc.exitStatus() != QProcess::NormalExit) {
        qFatal("Failed to download certificate %s from LDAP server.", qPrintable(certName));
    }

    // primitive LDIF parser, would be nicer with something like KLDAP
    const auto certLdif = QString::fromUtf8(proc.readAllStandardOutput());
    QRegularExpression regExp(QStringLiteral("cACertificate:: ([\\w\\W]*?)\n[^ ]"));
    const auto match = regExp.match(certLdif);
    const auto certData = match.captured(1).remove(QLatin1Char('\n')).remove(QLatin1Char(' ')).toUtf8();

    QFile f(certName + QLatin1String(".vdv-cert"));
    f.open(QFile::WriteOnly);
    f.write(QByteArray::fromBase64(certData));
}

static void writeQrc(const std::vector<QString> &certNames)
{
    QFile qrc(QStringLiteral("vdv-certs.qrc"));
    if (!qrc.open(QFile::WriteOnly)) {
        qFatal("Failed to open file %s: %s", qPrintable(qrc.fileName()), qPrintable(qrc.errorString()));
    }
    qrc.write(R"(<!--
    SPDX-FileCopyrightText: none
    SPDX-License-Identifier: CC0-1.0
-->
<RCC>
    <qresource prefix="/org.kde.pim/kitinerary/vdv/certs">
)");
    for (const auto &certName : certNames) {
        qrc.write("        <file>");
        qrc.write(certName.toUtf8());
        qrc.write(".vdv-cert</file>\n");
    }
    qrc.write("    </qresource>\n</RCC>\n");
}

static VdvCertificate loadCert(const QString &certName)
{
    QFile f(certName + QLatin1String(".vdv-cert"));
    if (!f.open(QFile::ReadOnly)) {
        qFatal("Failed to open file %s: %s", qPrintable(f.fileName()), qPrintable(f.errorString()));
    }
    return VdvCertificate(f.readAll());
}

static void decodeCert(const QString &certName)
{
    auto cert = loadCert(certName);
    if (cert.needsCaKey()) {
        qDebug() << certName << "needs decoding";
        const auto rootCa = loadCert(QStringLiteral("4555564456100106"));
        cert.setCaCertificate(rootCa);
        if (cert.isValid()) {
            QFile f(certName + QLatin1String(".vdv-cert"));
            if (!f.open(QFile::WriteOnly)) {
                qFatal("Failed to open file %s: %s", qPrintable(f.fileName()), qPrintable(f.errorString()));
            }
            cert.writeKey(&f);
        } else {
            qFatal("Decoding failed for %s", qPrintable(certName));;
        }
    } else if (cert.isValid()) {
        // this removes the signature and other unknown elements, leaving just the key
        QFile f(certName + QLatin1String(".vdv-cert"));
        if (!f.open(QFile::WriteOnly)) {
            qFatal("Failed to open file %s: %s", qPrintable(f.fileName()), qPrintable(f.errorString()));
        }
        cert.writeKey(&f);
    } else {
        qFatal("%s is invalid", qPrintable(certName));
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    // (1) list all certificates
    auto certNames = listCerts();

    // (2) load all certificates we don't have yet
    for (auto it = certNames.begin(); it != certNames.end();) {
        if (QFile::exists(QLatin1Char('.') + (*it) + QLatin1String(".vdv-cert"))) {
            // expired certificate, but cached from previous run
            it = certNames.erase(it);
            continue;
        }
        qDebug() << "checking certificate" << (*it);
        if (!QFile::exists((*it) + QLatin1String(".vdv-cert"))) {
            downloadCert(*it);
        }
        ++it;
    }

    // (3) decode certificates (avoids runtime cost and shrinks the file size)
    for (const auto &certName : certNames) {
        decodeCert(certName);
    }

    // (4) discard old sub-CA certificates we don't need
    for (auto it = certNames.begin(); it != certNames.end();) {
        const auto cert = loadCert(*it);
        if (!cert.isValid()) {
            qFatal("Invalid certificate: %s", qPrintable(*it));
        }
        if (!cert.isSelfSigned() && cert.endOfValidity().year() < 2019) {
            qDebug() << "discarding" << (*it) << "due to being expired" << cert.endOfValidity();
            QFile::rename((*it) + QLatin1String(".vdv-cert"), QLatin1Char('.') + (*it) + QLatin1String(".vdv-cert"));
            it = certNames.erase(it);
        } else {
            ++it;
        }
    }

    // (5) write qrc file
    std::sort(certNames.begin(), certNames.end());
    writeQrc(certNames);

    return 0;
}
