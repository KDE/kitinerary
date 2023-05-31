/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kitinerary_version.h>

#include <../lib/plist/plistreader.cpp>
#include <../lib/plist/plistdata_p.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>

#include <iostream>

using namespace KItinerary;

void dumpValue(const QVariant &v)
{
    if (v.userType() == qMetaTypeId<PListArray>()) {
        const auto a = v.value<PListArray>();
        std::cout << "[";
        for (uint64_t i = 0; i < a.size(); ++i) {
            std::cout << a.value(i);
            if (i != a.size() - 1) {
                std::cout << ",";
            }
        }
        std::cout << "]";
        return;
    }

    if (v.userType() == qMetaTypeId<PListDict>()) {
        const auto a = v.value<PListDict>();
        std::cout << "{";
        for (uint64_t i = 0; i < a.size(); ++i) {
            std::cout << a.key(i) << ":" << a.value(i);
            if (i != a.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "}";
        return;
    }

    if (v.userType() == qMetaTypeId<PListUid>()) {
        std::cout << "UID:" << v.value<PListUid>().value;
        return;
    }

    std::cout << qPrintable(v.toString());
}

static void writeIndent(int indent)
{
    while (indent) {
        std::cout << "  ";
        --indent;
    }
}

static void dumpRecursive(const PListReader &reader, uint64_t idx, int indent = 0)
{
    const auto type = reader.objectType(idx);
    switch (type) {
        case PListObjectType::Array:
        {
            const auto a = reader.object(idx).value<PListArray>();
            std::cout << "[" << std::endl;
            for (uint64_t i = 0; i < a.size(); ++i) {
                writeIndent(indent + 1);
                dumpRecursive(reader, a.value(i), indent + 1);
            }
            writeIndent(indent);
            std::cout << "]" << std::endl;
            break;
        }
        case PListObjectType::Dict:
        {
            const auto d = reader.object(idx).value<PListDict>();
            std::cout << "{" << std::endl;
            for (uint64_t i = 0; i < d.size(); ++i) {
                writeIndent(indent + 1);
                dumpValue(reader.object(d.key(i)));
                std::cout << ": ";
                dumpRecursive(reader, d.value(i), indent + 1);
            }
            writeIndent(indent);
            std::cout << "}" << std::endl;
            break;
        }
        default:
            dumpValue(reader.object(idx));
            std::cout << std::endl;
    }
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("plist-dump"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Dump binary plist content."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("File to read data from, omit for using stdin."));
    parser.process(app);

    QFile file;
    if (parser.positionalArguments().isEmpty()) {
        file.open(stdin, QFile::ReadOnly);
    } else {
        file.setFileName(parser.positionalArguments().at(0));
        if (!file.open(QFile::ReadOnly)) {
            std::cerr << qPrintable(file.errorString()) << std::endl;
            return 1;
        }
    }

    const auto buffer = file.readAll();
    PListReader reader(buffer);

    // object list
    for (uint64_t i = 0; i < reader.objectCount(); ++i) {
        std::cout << i << ": [" << (int)reader.objectType(i) << "] ";
        dumpValue(reader.object(i));
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;

    // object tree
    dumpRecursive(reader, reader.rootObjectIndex());
    std::cout << std::endl << std::endl;

    // NSKeyedArchiver
    std::cout << QJsonDocument(reader.unpackKeyedArchive().toObject()).toJson().constData() << std::endl;
}

