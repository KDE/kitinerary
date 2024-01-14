/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "action.h"
#include "datatypes_p.h"

#include <QUrl>

using namespace KItinerary;

namespace KItinerary {

class ActionPrivate : public QSharedData
{
    KITINERARY_PRIVATE_BASE_GADGET(Action)
public:
    QUrl target;
    QVariant result;
};

KITINERARY_MAKE_CLASS(Action)
KITINERARY_MAKE_PROPERTY(Action, QUrl, target, setTarget)
KITINERARY_MAKE_PROPERTY(Action, QVariant, result, setResult)
KITINERARY_MAKE_OPERATOR(Action)

class CancelActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(CancelAction)
};
KITINERARY_MAKE_DERIVED_CLASS(CancelAction, Action)
KITINERARY_MAKE_OPERATOR(CancelAction)

class CheckInActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(CheckInAction)
};
KITINERARY_MAKE_DERIVED_CLASS(CheckInAction, Action)
KITINERARY_MAKE_OPERATOR(CheckInAction)

class DownloadActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(DownloadAction)
};
KITINERARY_MAKE_DERIVED_CLASS(DownloadAction, Action)
KITINERARY_MAKE_OPERATOR(DownloadAction)

class JoinActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(JoinAction)
};
KITINERARY_MAKE_DERIVED_CLASS(JoinAction, Action)
KITINERARY_MAKE_OPERATOR(JoinAction)

class ReserveActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(ReserveAction)
};
KITINERARY_MAKE_DERIVED_CLASS(ReserveAction, Action)
KITINERARY_MAKE_OPERATOR(ReserveAction)

class UpdateActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(UpdateAction)
};
KITINERARY_MAKE_DERIVED_CLASS(UpdateAction, Action)
KITINERARY_MAKE_OPERATOR(UpdateAction)

class ViewActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(ViewAction)
};
KITINERARY_MAKE_DERIVED_CLASS(ViewAction, Action)
KITINERARY_MAKE_OPERATOR(ViewAction)

}

template <>
KItinerary::ActionPrivate *QExplicitlySharedDataPointer<KItinerary::ActionPrivate>::clone()
{
    return d->clone();
}

#include "moc_action.cpp"
