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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_ACTION_H
#define KITINERARY_ACTION_H

#include "kitinerary_export.h"
#include "datatypes.h"

class QUrl;

namespace KItinerary {

class ActionPrivate;

/** Base class for actions.
 *  @see https://schema.org/Action
 */
class KITINERARY_EXPORT Action
{
    KITINERARY_BASE_GADGET(Action)
    KITINERARY_PROPERTY(QUrl, target, setTarget)
protected:
    ///@cond internal
    QExplicitlySharedDataPointer<ActionPrivate> d;
    ///@endcond
};

/** Cancel action.
 *  @see https://schema.org/CancelAction
 */
class KITINERARY_EXPORT CancelAction : public Action
{
    KITINERARY_GADGET(CancelAction)
};

/** Check-in action.
 *  @see https://schema.org/CheckInAction
 */
class KITINERARY_EXPORT CheckInAction : public Action
{
    KITINERARY_GADGET(CheckInAction)
};

/** Download action.
 *  @see https://schema.org/DownloadAction
 */
class KITINERARY_EXPORT DownloadAction : public Action
{
    KITINERARY_GADGET(DownloadAction)
};

/** Reserve action.
 *  @see https://schema.org/ReserveAction
 */
class KITINERARY_EXPORT ReserveAction : public Action
{
    KITINERARY_GADGET(ReserveAction)
};

/** Edit/update action.
 *  @see https://schema.org/UpdateAction
 */
class KITINERARY_EXPORT UpdateAction : public Action
{
    KITINERARY_GADGET(UpdateAction)
};

/** View action.
 *  @see https://schema.org/ViewAction
 */
class KITINERARY_EXPORT ViewAction : public Action
{
    KITINERARY_GADGET(ViewAction)
};

}

#endif // KITINERARY_ACTION_H
