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

#ifndef EXTRACTORRULE_H
#define EXTRACTORRULE_H

#include <QRegularExpression>
#include <QString>
#include <QVector>

class ExtractorContext;
class QXmlStreamReader;

/** A single unstructured data extractor rule. */
class ExtractorRule
{
public:
    virtual ~ExtractorRule();
    virtual bool load(QXmlStreamReader &reader);
    virtual bool match(ExtractorContext *context) const = 0;

    QString name() const;
    QString type() const;
    bool repeats() const;

protected:
    QString value(const QRegularExpressionMatch &match, ExtractorContext *context) const;
    QString format() const;

    QRegularExpression m_regexp;

private:
    QString m_name;
    QString m_type;
    QString m_value;
    QString m_format;
    bool m_repeat = false;
};

class ExtractorVariableRule : public ExtractorRule
{
public:
    bool match(ExtractorContext *context) const override;
};

class ExtractorClassRule : public ExtractorRule
{
public:
    ~ExtractorClassRule();
    bool load(QXmlStreamReader &reader) override;
    bool match(ExtractorContext *context) const override;
    QVector<ExtractorRule *> rules() const;
private:
    QVector<ExtractorRule *> m_rules;
};

class ExtractorPropertyRule : public ExtractorRule
{
public:
    bool match(ExtractorContext *context) const override;
};

#endif // EXTRACTORRULE_H
