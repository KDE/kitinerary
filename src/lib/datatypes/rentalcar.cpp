/*
    SPDX-FileCopyrightText: 2018-2022 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rentalcar.h"
#include "datatypes_p.h"

#include <QDateTime>
#include <QUrl>

using namespace KItinerary;

namespace KItinerary {

class RentalCarPrivate: public QSharedData {

public:
    QString name;
    QString model;
    Organization rentalCompany;
    Brand brand;
};

KITINERARY_MAKE_SIMPLE_CLASS(RentalCar)
KITINERARY_MAKE_PROPERTY(RentalCar, QString, name, setName)
KITINERARY_MAKE_PROPERTY(RentalCar, QString, model, setModel)
KITINERARY_MAKE_PROPERTY(RentalCar, Organization, rentalCompany, setRentalCompany)
KITINERARY_MAKE_PROPERTY(RentalCar, Brand, brand, setBrand)
KITINERARY_MAKE_OPERATOR(RentalCar)
}

#include "moc_rentalcar.cpp"
