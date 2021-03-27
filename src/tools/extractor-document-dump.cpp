/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kitinerary_version.h>

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <iostream>

using namespace KItinerary;

static void printNode(const ExtractorDocumentNode &node, int indent = 0)
{
    if (node.isNull()) {
        return;
    }
    for (int j = 0; j < indent; ++j) { std::cout.write(" ", 1); }
    std::cout << "MIME type: " << qPrintable(node.mimeType()) << std::endl;
    for (int j = 0; j < indent; ++j) { std::cout.write(" ", 1); }
    std::cout << "context time : " << qPrintable(node.contextDateTime().toString(Qt::ISODate)) << std::endl;
    if (!node.location().isNull()) {
        for (int j = 0; j < indent; ++j) { std::cout.write(" ", 1); }
        std::cout << "location: " << qPrintable(node.location().toString()) << std::endl;
    }
    for (int j = 0; j < indent; ++j) { std::cout.write(" ", 1); }
    std::cout << "content: " << node.content().typeName() << std::endl;
    for (int j = 0; j < indent; ++j) { std::cout.write(" ", 1); }
    std::cout << "results: " << node.result().size() << std::endl;

    for (const auto &child : node.childNodes()) {
        printNode(child, indent + 2);
    }
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("extractor-document-dump"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Dump extractor document node tree."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("File to read data from, omit for using stdin."));
    parser.process(app);

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    QFile file(parser.positionalArguments().at(0));
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << qPrintable(file.errorString()) << std::endl;
        return 1;
    }

    const auto data = file.readAll();

    ExtractorEngine engine;
    engine.setData(data, file.fileName());
    engine.extract();
    printNode(engine.rootDocumentNode());

    return 0;
}
