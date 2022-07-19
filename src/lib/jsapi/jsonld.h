/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QObject>

class QJSEngine;
class QJSValue;

namespace KItinerary {
namespace JsApi {

/** Methods to create JSON-LD objects. */
class JsonLd : public QObject
{
    Q_OBJECT
public:
    ///@cond internal
    explicit JsonLd(QJSEngine *engine);
    ~JsonLd();
    ///@endcond

    /** Create a new JSON-LD object of type @p typeName. */
    Q_INVOKABLE QJSValue newObject(const QString &typeName) const;

    /** Convenience method that generates a full FlightReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newFlightReservation() const;
    /** Convenience method that generates a full TrainReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newTrainReservation() const;
    /** Convenience method that generates a full BusReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newBusReservation() const;
    /** Convenience method that generates a full LodgingReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newLodgingReservation() const;
    /** Convenience method that generates a full EventReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newEventReservation() const;
    /** Convenience method that generates a full FoodEstablishmentReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newFoodEstablishmentReservation() const;
    /** Convenience method that generates a full RentalCarReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newRentalCarReservation() const;
    /** Convenience method that generates a full BoatReservation JS object.
     *  This can be used by extractor scripts to fill in the extracted information.
     */
    Q_INVOKABLE QJSValue newBoatReservation() const;

    /** Convert a train reservation to a bus reservation. */
    Q_INVOKABLE QJSValue trainToBusReservation(const QJSValue &trainRes) const;
    /** Convert a bus reservation to a train reservation. */
    Q_INVOKABLE QJSValue busToTrainReservation(const QJSValue &busRes) const;

    /** Convert a date/time string to a date/time value.
     *  @param dtStr The input string containing a date/time value.
     *  @param format The format of the input string. Same format specification as
     *  used by QLocale and QDateTime. If the year is not part of the date
     *  it is attempted to be recovered from the context date set on the
     *  ExtractorEngine (that is, the returned date will be after the context
     *  date). Can be a string or an array of strings, which are then tried sequentially.
     *  @param localeName The locale in which the string is formatted. This is
     *  relevant when the input contains for example localized month names or
     *  month abbreviations. Can be a string or an array of strings.
     */
    Q_INVOKABLE QDateTime toDateTime(const QString &dtStr, const QJSValue &format, const QJSValue &localeName) const;
    /** Convert object @p v to a JSON-LD object.
     *  This is useful when interacting with API returning regular data types,
     *  such as Uic9183Parser.
     */
    Q_INVOKABLE QJSValue toJson(const QVariant &v) const;
    /** Clones the given JS object.
     *  That is, create a deep copy of @p v.
     */
    Q_INVOKABLE QJSValue clone(const QJSValue &v) const;
    /** Parses geo coordinates from a given mapping service URLs.
     *  This consumes for example Google Maps links and returns a JSON-LD
     *  GeoCoordinates object.
     */
    Q_INVOKABLE QJSValue toGeoCoordinates(const QString &mapUrl);

    /** Read a QDateTime property and return a JSON-LD serialization of it.
     *  This is a workaround for JS destroying timezone information when getting in touch with a QDateTime
     *  object. With this method it is safe to read a QDateTime property e.g. from a Qt gadget or QObject
     *  without the risk of losing information.
     *  @param obj The object to read from.
     *  @param propName The name of the property to read.
     */
    Q_INVOKABLE QJSValue readQDateTime(const QVariant &obj, const QString &propName) const;

    /** @see JsonLdDocument::apply. */
    Q_INVOKABLE QJSValue apply(const QJSValue &lhs, const QJSValue &rhs) const;

    ///@cond internal
    void setContextDate(const QDateTime &dt);
    ///@endcond
private:
    QJSValue newPlace(const QString &type) const;
    QDateTime toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const;
    QJSEngine *m_engine;
    QDateTime m_contextDate;
};

}
}

