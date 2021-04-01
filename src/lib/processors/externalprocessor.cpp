/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>

#include "externalprocessor.h"
#include "logging.h"

#include <KItinerary/AbstractExtractor>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorRepository>
#include <KItinerary/ExtractorResult>
#include <KItinerary/PdfDocument>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>

using namespace KItinerary;

ExternalProcessor::ExternalProcessor()
{
    // find external extractor
    QFileInfo fi(QLatin1String(CMAKE_INSTALL_FULL_LIBEXECDIR_KF5) + QLatin1String("/kitinerary-extractor"));
    if (!fi.exists() && !fi.isFile() && !fi.isExecutable()) {
        qCCritical(Log) << "Cannot find external extractor:" << fi.fileName();
        return;
    }
    m_externalExtractor = fi.canonicalFilePath();
}

ExternalProcessor::~ExternalProcessor() = default;

bool ExternalProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return PdfDocument::maybePdf(encodedData) || fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive);
}

ExtractorDocumentNode ExternalProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(encodedData);
    node.setMimeType(QStringLiteral("application/pdf"));
    return node;
}

void ExternalProcessor::preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    std::vector<const AbstractExtractor*> extractors;
    engine->extractorRepository()->extractorsForNode(node, extractors);
    // consider the implicit conversion to text/plain the PDF processor can do
    if (node.mimeType() == QLatin1String("application/pdf")) {
        node.setMimeType(QStringLiteral("text/plain"));
        engine->extractorRepository()->extractorsForNode(node, extractors);
        node.setMimeType(QStringLiteral("application/pdf"));
    }

    QStringList extNames;
    extNames.reserve(extractors.size());
    std::transform(extractors.begin(), extractors.end(), std::back_inserter(extNames), [](auto ext) { return ext->name(); });

    QProcess proc;
    proc.setProgram(m_externalExtractor);

    QStringList args({QLatin1String("--context-date"), node.contextDateTime().toString(Qt::ISODate),
                      QLatin1String("--extractors"), extNames.join(QLatin1Char(';')),
                      QLatin1String("--no-validation")});
    const auto extraPaths = engine->extractorRepository()->additionalSearchPaths();
    for (const auto &p : extraPaths) {
        args.push_back(QStringLiteral("--additional-search-path"));
        args.push_back(p);
    }

    proc.setArguments(args);
    proc.start(QProcess::ReadWrite);
    proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    if (!proc.waitForStarted(1000)) {
        qCWarning(Log) << "could not start external extractor" << m_externalExtractor << proc.errorString();
        return;
    }
    proc.write(node.content<QByteArray>());
    proc.closeWriteChannel();
    if (!proc.waitForFinished(15000)) {
        qCWarning(Log) << "external extractor did not exit cleanly" << m_externalExtractor << proc.errorString();
        return;
    }

    const auto res = QJsonDocument::fromJson(proc.readAllStandardOutput()).array();
    node.addResult(res);
}
