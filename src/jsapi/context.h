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

#ifndef KITINERARY_JSAPI_CONTEXT_H
#define KITINERARY_JSAPI_CONTEXT_H

#include <QDateTime>
#include <QObject>

namespace KItinerary {

/** JavaScript API exposed to extractor scripts. */
namespace JsApi {

/** The extraction context.
 *  This object contains information about what is being extracted,
 *  or where the extracted information is coming from.
 */
class Context : public QObject
{
    Q_OBJECT
    /** The time the email containing the extracted data was sent.
     *  This can be useful if the extracted data only contains dates without
     *  specifying a year. The year can then be infered out of this context.
     */
    Q_PROPERTY(QDateTime senderDate MEMBER m_senderDate)

public:
    ///@cond internal
    QDateTime m_senderDate;
    ///@endcond
};

}
}

#endif // KITINERARY_JSAPI_CONTEXT_H
