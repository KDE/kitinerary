/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    KITINERARY_GADGET(Action)
    KITINERARY_PROPERTY(QUrl, target, setTarget)
    KITINERARY_PROPERTY(QVariant, result, setResult)
protected:
    ///@cond internal
    QExplicitlySharedDataPointer<ActionPrivate> d;
    ///@endcond
};

class CancelActionPrivate;

/** Cancel action.
 *  @see https://schema.org/CancelAction
 */
class KITINERARY_EXPORT CancelAction : public Action
{
    KITINERARY_GADGET(CancelAction)
};

class CheckInActionPrivate;

/** Check-in action.
 *  @see https://schema.org/CheckInAction
 */
class KITINERARY_EXPORT CheckInAction : public Action
{
    KITINERARY_GADGET(CheckInAction)
};

class DownloadActionPrivate;

/** Download action.
 *  @see https://schema.org/DownloadAction
 */
class KITINERARY_EXPORT DownloadAction : public Action
{
    KITINERARY_GADGET(DownloadAction)
};

class JoinActionPrivate;

/** Action to join an event.
 *  @see https://schema.org/JoinAction
 */
class KITINERARY_EXPORT JoinAction : public Action
{
    KITINERARY_GADGET(JoinAction)
};

class ReserveActionPrivate;

/** Reserve action.
 *  @see https://schema.org/ReserveAction
 */
class KITINERARY_EXPORT ReserveAction : public Action
{
    KITINERARY_GADGET(ReserveAction)
};

class UpdateActionPrivate;

/** Edit/update action.
 *  @see https://schema.org/UpdateAction
 */
class KITINERARY_EXPORT UpdateAction : public Action
{
    KITINERARY_GADGET(UpdateAction)
};

class ViewActionPrivate;

/** View action.
 *  @see https://schema.org/ViewAction
 */
class KITINERARY_EXPORT ViewAction : public Action
{
    KITINERARY_GADGET(ViewAction)
};

}

