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

#ifndef EXTRACTORPREPROCESSOR_H
#define EXTRACTORPREPROCESSOR_H

#include "kitinerary_export.h"

#include <QString>

/** Preprocessing of HTML and PDF attachments. */
class KITINERARY_EXPORT ExtractorPreprocessor
{
public:
    void preprocessPlainText(const QString &input);
    void preprocessHtml(const QString &input);
    void preprocessPdf(const QByteArray &input);

    QString text() const;

private:
    void replaceEntityAndAppend(const QStringRef &source);
    QString m_buffer;
};

#endif // EXTRACTORPREPROCESSOR_H
