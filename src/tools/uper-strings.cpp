/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kitinerary_version.h>

#include <../lib/asn1/bitvectorview.cpp>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>

#include <iostream>

using namespace KItinerary;

static bool isPlausibleChar(uint8_t c)
{
    return c >= 0x20 && !std::iscntrl(c);
}

struct Result {
    BitVectorView::size_type offset;
    QByteArray::size_type length;
    QByteArray data;
};

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("uper-strings"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Find strings in uPER encoded content."));
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
    const auto view = BitVectorView(std::string_view(buffer.constData(), buffer.size()));

    // search for IA5Strings
    std::vector<Result> results;
    QByteArray candidate;
    for (int i = 0; i < 7; ++i) {
        BitVectorView::size_type j = i;
        for (; j < view.size() - 7; j += 7) {
            auto c = view.valueAtMSB<uint8_t>(j, 7);
            if (isPlausibleChar(c)) {
                candidate.push_back((char)c);
                continue;
            }

            if (candidate.size() <= 2) {
                candidate.clear();
                continue;
            }

            results.push_back({j - candidate.size() * 7, candidate.size() * 7, candidate});
            candidate.clear();
        }
        if (candidate.size() > 2) {
            results.push_back({j - candidate.size() * 7, candidate.size() * 7, candidate});
        }
        candidate.clear();
    }

    std::sort(results.begin(), results.end(), [](const auto &lhs, const auto &rhs) { return lhs.offset < rhs.offset; });
    std::cout << "IA5String candidates:" << std::endl;
    for (const auto &res : results) {
        std::cout << res.offset << ": " << res.data.constData() << std::endl;
    }

    // search for UTF8String
    results.clear();
    for (int i = 0; i < 8; ++i) {
        BitVectorView::size_type j = i;
        for (; j < view.size() - 8; j += 8) {
            auto c = view.valueAtMSB<uint8_t>(j, 8);
            if (isPlausibleChar(c)) {
                candidate.push_back((char)c);
                continue;
            }

            if (candidate.size() <= 2) {
                candidate.clear();
                continue;
            }

            results.push_back({j - candidate.size() * 8, candidate.size() * 8, candidate});
            candidate.clear();
        }
        if (candidate.size() > 2) {
            results.push_back({j - candidate.size() * 8, candidate.size() * 8, candidate});
        }
        candidate.clear();
    }

    std::sort(results.begin(), results.end(), [](const auto &lhs, const auto &rhs) { return lhs.offset < rhs.offset; });
    std::cout << std::endl << "UTF8String candidates:" << std::endl;
    for (const auto &res : results) {
        std::cout << res.offset << ": " << qPrintable(QString::fromUtf8(res.data.constData())) << std::endl;
    }
}
