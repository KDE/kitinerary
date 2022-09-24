/*
    SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include <QFile>
#include <QJsonDocument>
#include <QProcess>
#include <QTest>

namespace Test {
    template <typename T>
    inline bool compareJson(const QString &refFile, const T &output, const T &ref)
    {
        if (output != ref) {
#ifndef Q_OS_WIN
            QFile failFile(refFile + QLatin1String(".fail"));
            failFile.open(QFile::WriteOnly);
            failFile.write(QJsonDocument(output).toJson());
            failFile.close();

            QProcess proc;
            proc.setProcessChannelMode(QProcess::ForwardedChannels);
            proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), refFile, failFile.fileName()});
            proc.waitForFinished();
#else
            qDebug().noquote() << "Got:" << QJsonDocument(output).toJson();
            qDebug().noquote() << "Expected:" << QJsonDocument(ref).toJson();
#endif
            return false;
        }
        return true;
    }
}

#endif
