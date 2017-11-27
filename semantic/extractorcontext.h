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

#ifndef EXTRACTORCONTEXT_H
#define EXTRACTORCONTEXT_H

#include <QHash>
#include <QJsonObject>
#include <QVector>

class ExtractorEngine;
class ExtractorRule;

class QJsonValue;

/** Context stack used inside ExtractorEngine. */
class ExtractorContext
{
public:
    ExtractorContext(ExtractorEngine *engine, ExtractorContext *parent = nullptr);
    ~ExtractorContext();

    ExtractorEngine *engine() const;
    QVector<ExtractorRule *> &rules();
    void setRules(const QVector<ExtractorRule *> &rules);

    int offset() const;
    void setOffset(int offset);

    QString variableValue(const QString &name) const;
    void setVariable(const QString &name, const QString &value);
    void setProperty(const QString &name, const QJsonValue &value);
    QJsonObject object() const;

private:
    ExtractorEngine *m_engine;
    ExtractorContext *m_parent;
    QVector<ExtractorRule *> m_rules;
    QHash<QString, QString> m_variables;
    QJsonObject m_obj;
    int m_offset = 0;
};

#endif // EXTRACTORCONTEXT_H
