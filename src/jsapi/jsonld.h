/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_JSAPI_JSONLD_H
#define KITINERARY_JSAPI_JSONLD_H

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
    /** Convert a date/time string to a date/time value.
     *  @param dtStr The input string containing a date/time value.
     *  @param format The format of the input string. Same format specification as
     *  used by QLocale and QDateTime. If the year is not part of the date
     *  it is attempted to be recovered from the context date set on the
     *  ExtractorEngine (that is, the returned date will be after the context
     *  date).
     *  @param localeName The locale in which the string is formatted. This is
     *  relevant when the input contains for example localized month names or
     *  month abbreviations.
     */
    Q_INVOKABLE QDateTime toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const;
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

    ///@cond internal
    void setContextDate(const QDateTime &dt);
    ///@endcond
private:
    QJSEngine *m_engine;
    QDateTime m_contextDate;
};

}
}

#endif // KITINERARY_JSAPI_JSONLD_H
