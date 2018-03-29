/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "extractorengine.h"
#include "extractor.h"
#include "logging.h"

#include <KPkPass/BoardingPass>

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QJSEngine>

using namespace KItinerary;

namespace KItinerary {

class ContextObject;

class ExtractorEnginePrivate {
public:
    void executeScript();

    const Extractor *m_extractor = nullptr;
    ContextObject *m_context = nullptr;
    QString m_text;
    KPkPass::BoardingPass *m_pass;
    QJsonArray m_result;
};

class JsApi : public QObject
{
    Q_OBJECT
public:
    explicit JsApi(QJSEngine *engine)
        : QObject(engine)
        , m_engine(engine)
    {
    }

    Q_INVOKABLE QJSValue newObject(const QString &typeName) const;
    Q_INVOKABLE QDateTime toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const;

private:
    QJSEngine *m_engine;
};

QJSValue JsApi::newObject(const QString &typeName) const
{
    auto v = m_engine->newObject();
    v.setProperty(QStringLiteral("@type"), typeName);
    return v;
}

QDateTime JsApi::toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const
{
    QLocale locale(localeName);
    const auto dt = locale.toDateTime(dtStr, format);
    if (dt.isValid()) {
        return dt;
    }

    // try harder for the "MMM" month format
    // QLocale expects the exact string in QLocale::shortMonthName(), while we often encounter a three
    // letter month identifier. For en_US that's the same, for Swedish it isn't though for example. So
    // let's try to fix up the month identifiers to the full short name.
    if (format.contains(QLatin1String("MMM"))) {
        auto dtStrFixed = dtStr;
        for (int i = 0; i < 12; ++i) {
            const auto monthName = locale.monthName(i, QLocale::ShortFormat);
            dtStrFixed = dtStrFixed.replace(monthName.left(3), monthName);
        }
        return locale.toDateTime(dtStrFixed, format);
    }
    return dt;
}

class ContextObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDateTime senderDate MEMBER m_senderDate)

public:
    QDateTime m_senderDate;
};
}

ExtractorEngine::ExtractorEngine()
    : d(new ExtractorEnginePrivate)
{
    d->m_context = new ContextObject; // will be deleted by QJSEngine taking ownership
}

ExtractorEngine::ExtractorEngine(ExtractorEngine &&) = default;
ExtractorEngine::~ExtractorEngine() = default;

void ExtractorEngine::setExtractor(const Extractor *extractor)
{
    d->m_extractor = extractor;
}

void ExtractorEngine::setText(const QString &text)
{
    d->m_text = text;
}

void ExtractorEngine::setPass(KPkPass::Pass *pass)
{
    d->m_pass = qobject_cast<KPkPass::BoardingPass*>(pass);
}

void ExtractorEngine::setSenderDate(const QDateTime &dt)
{
    d->m_context->m_senderDate = dt;
}

QJsonArray ExtractorEngine::extract()
{
    if (!d->m_extractor) {
        return {};
    }
    switch (d->m_extractor->type()) {
        case Extractor::Text:
            if (d->m_text.isEmpty()) {
                return {};
            }
            break;
        case Extractor::PkPass:
            if (!d->m_pass) {
                return {};
            }
            break;
    }

    d->executeScript();
    return d->m_result;
}

void ExtractorEnginePrivate::executeScript()
{
    Q_ASSERT(m_extractor);

    QFile f(m_extractor->scriptFileName());
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open extractor script" << f.fileName() << f.errorString();
        return;
    }

    QJSEngine engine;
    engine.installExtensions(QJSEngine::ConsoleExtension);
    auto jsApi = new JsApi(&engine);
    engine.globalObject().setProperty(QStringLiteral("JsonLd"), engine.newQObject(jsApi));
    engine.globalObject().setProperty(QStringLiteral("Context"), engine.newQObject(m_context));
    auto result = engine.evaluate(QString::fromUtf8(f.readAll()), f.fileName());
    if (result.isError()) {
        qCWarning(Log) << "Script parsing error in" << result.property(QLatin1String("fileName")).toString()
                                << ':' << result.property(QLatin1String("lineNumber")).toInt() << result.toString();
        return;
    }

    auto mainFunc = engine.globalObject().property(QLatin1String("main"));
    if (!mainFunc.isCallable()) {
        qCWarning(Log) << "Script has no main() function!";
        return;
    }

    QJSValueList args;
    switch (m_extractor->type()) {
        case Extractor::Text:
            args = {m_text};
            break;
        case Extractor::PkPass:
            args = {engine.toScriptValue<QObject*>(m_pass)};
            break;
    }

    result = mainFunc.call(args);
    if (result.isError()) {
        qCWarning(Log) << "Script execution error in" << result.property(QLatin1String("fileName")).toString()
                                << ':' << result.property(QLatin1String("lineNumber")).toInt() << result.toString();
        return;
    }

    if (result.isArray()) {
        m_result = QJsonArray::fromVariantList(result.toVariant().toList());
    } else if (result.isObject()) {
        m_result = { QJsonValue::fromVariant(result.toVariant()) };
    } else {
        qCWarning(Log) << "Invalid result type from script";
    }
}

#include "extractorengine.moc"
