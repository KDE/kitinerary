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

#ifndef EXTRACTORPOSTPROCESSOR_H
#define EXTRACTORPOSTPROCESSOR_H

#include "kitinerary_export.h"

#include <QVariant>
#include <QVector>

#include <memory>

namespace KItinerary {

class ExtractorPostprocessorPrivate;

/** Post-process extracted data to filter out garbage and augment data from other sources. */
class KITINERARY_EXPORT ExtractorPostprocessor
{
public:
    ExtractorPostprocessor();
    ExtractorPostprocessor(const ExtractorPostprocessor&) = delete;
    ExtractorPostprocessor(ExtractorPostprocessor&&);
    ~ExtractorPostprocessor();

    void process(const QVector<QVariant> &data);
    QVector<QVariant> result() const;

    /** The date the reservation(s) processed here have been made, if known.
     *  This is used for determining the year of incomplete dates provided by
     *  various sources. Therefore this has to be somewhen before the reservation
     *  becomes due.
     */
    void setContextDate(const QDateTime &dt);

private:
    std::unique_ptr<ExtractorPostprocessorPrivate> d;
};

}

#endif // EXTRACTORPOSTPROCESSOR_H
