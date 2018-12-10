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
};

KITINERARY_MAKE_BASE_CLASS(Action)
KITINERARY_MAKE_PROPERTY(Action, QUrl, target, setTarget)
KITINERARY_MAKE_OPERATOR(Action)

class CancelActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(CancelAction)
};
KITINERARY_MAKE_SUB_CLASS(CancelAction, Action)
KITINERARY_MAKE_OPERATOR(CancelAction)

class CheckInActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(CheckInAction)
};
KITINERARY_MAKE_SUB_CLASS(CheckInAction, Action)
KITINERARY_MAKE_OPERATOR(CheckInAction)

class DownloadActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(DownloadAction)
};
KITINERARY_MAKE_SUB_CLASS(DownloadAction, Action)
KITINERARY_MAKE_OPERATOR(DownloadAction)

class UpdateActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(UpdateAction)
};
KITINERARY_MAKE_SUB_CLASS(UpdateAction, Action)
KITINERARY_MAKE_OPERATOR(UpdateAction)

class ViewActionPrivate : public ActionPrivate
{
    KITINERARY_PRIVATE_GADGET(ViewAction)
};
KITINERARY_MAKE_SUB_CLASS(ViewAction, Action)
KITINERARY_MAKE_OPERATOR(ViewAction)

}

template <>
KItinerary::ActionPrivate *QExplicitlySharedDataPointer<KItinerary::ActionPrivate>::clone()
{
    return d->clone();
}

#include "moc_action.cpp"
