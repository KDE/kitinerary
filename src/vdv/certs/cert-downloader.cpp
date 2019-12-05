/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <vdvcertificate_p.h>

#include <QCoreApplication>
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
    qrc.write("<RCC>\n    <qresource prefix=\"/org.kde.pim/kitinerary/vdv/certs\">\n");
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
    for (const auto &certName : certNames) {
        qDebug() << "checking certificate" << certName;
        if (QFile::exists(certName + QLatin1String(".vdv-cert"))) {
            continue;
        }
        downloadCert(certName);
    }

    // (3) decode certificates (avoids runtime cost and shrinks the file size)
    for (const auto &certName : certNames) {
        decodeCert(certName);
    }

    // (4) discard old sub-CA certificates we don't need
    // TODO

    // (5) write qrc file
    std::sort(certNames.begin(), certNames.end());
    writeQrc(certNames);

    return 0;
}
