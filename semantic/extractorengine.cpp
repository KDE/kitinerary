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
#include "semantic_debug.h"

#include <QDateTime>
#include <QFile>
#include <QLocale>
#include <QJSEngine>

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
    Q_INVOKABLE QDateTime toDateTime(const QString &dt, const QString &format, const QString &locale) const;

private:
    QJSEngine *m_engine;
};

QJSValue JsApi::newObject(const QString &typeName) const
{
    auto v = m_engine->newObject();
    v.setProperty(QStringLiteral("@type"), typeName);
    return v;
}

QDateTime JsApi::toDateTime(const QString &dt, const QString &format, const QString &locale) const
{
    return QLocale(locale).toDateTime(dt, format);
}

ExtractorEngine::ExtractorEngine() = default;
ExtractorEngine::~ExtractorEngine() = default;

void ExtractorEngine::setExtractor(const Extractor *extractor)
{
    m_extractor = extractor;
}

const QString &ExtractorEngine::text() const
{
    return m_text;
}

void ExtractorEngine::setText(const QString &text)
{
    m_text = text;
}

QJsonArray ExtractorEngine::extract()
{
    if (!m_extractor || m_text.isEmpty()) {
        return {};
    }

    executeScript();
    return m_result;
}

void ExtractorEngine::executeScript()
{
    Q_ASSERT(m_extractor);

    QFile f(m_extractor->scriptFileName());
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(SEMANTIC_LOG) << "Failed to open extractor script" << f.fileName() << f.errorString();
        return;
    }

    QJSEngine engine;
    engine.installExtensions(QJSEngine::ConsoleExtension);
    auto jsApi = new JsApi(&engine);
    engine.globalObject().setProperty(QStringLiteral("JsonLd"), engine.newQObject(jsApi));
    auto result = engine.evaluate(QString::fromUtf8(f.readAll()), f.fileName());
    if (result.isError()) {
        qCWarning(SEMANTIC_LOG) << "Script parsing error in" << result.property(QLatin1String("fileName")).toString()
                                << ':' << result.property(QLatin1String("lineNumber")).toInt() << result.toString();
        return;
    }

    auto mainFunc = engine.globalObject().property(QLatin1String("main"));
    if (!mainFunc.isCallable()) {
        qCWarning(SEMANTIC_LOG) << "Script has no main() function!";
        return;
    }
    result = mainFunc.call({m_text});
    if (result.isError()) {
        qCWarning(SEMANTIC_LOG) << "Script execution error in" << result.property(QLatin1String("fileName")).toString()
                                << ':' << result.property(QLatin1String("lineNumber")).toInt() << result.toString();
        return;
    }

    m_result = QJsonArray::fromVariantList(result.toVariant().toList());
}

#include "extractorengine.moc"
