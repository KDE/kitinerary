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

#include <QVariant>
#include <QVector>

/** Post-process extracted data to filter out garbage and augment data from other sources. */
class ExtractorPostprocessor
{
public:
    void process(const QVector<QVariant> &data);
    QVector<QVariant> result() const;

private:
    QVariant processProperty(QVariant obj, const char *name, QVariant (ExtractorPostprocessor::*processor)(QVariant) const) const;

    QVariant processFlightReservation(QVariant res) const;
    QVariant processFlight(QVariant flight) const;
    QVariant processAirport(QVariant airport) const;
    void processFlightTime(QVariant &flight, const char *timePropName, const char *airportPropName) const;

    bool filterReservation(const QVariant &res) const;
    bool filterFlight(const QVariant &flight) const;
    bool filterAirport(const QVariant &airport) const;
    bool filterTrainTrip(const QVariant &trip) const;
    bool filterTrainStation(const QVariant &station) const;

    static QDateTime startDateTime(const QVariant &res);

    QVector<QVariant> m_data;
};

#endif // EXTRACTORPOSTPROCESSOR_H
