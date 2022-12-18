/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include "extractorcapabilities.h"

#include <KItinerary/ExtractorRepository>

#include <kcalendarcore_version.h>

#include <QString>

using namespace KItinerary;

QString ExtractorCapabilities::capabilitiesString()
{
    const char s[] =
        "Engine version      : " KITINERARY_VERSION_STRING "\n"
        "Qt version          : " QT_VERSION_STR "\n"

        "HTML support        : "
#if HAVE_LIBXML2
              "libxml2"
#else
              "not available"
#endif
        "\n"

        "PDF support         : poppler (" KPOPPLER_VERSION_STRING ")\n"

        "iCal support        : kcal (" KCALENDARCORE_VERSION_STRING ")\n"

        "Barcode decoder     : ZXing (" ZXING_VERSION_STRING ")\n"

        "Phone number decoder: "
#if HAVE_PHONENUMBER
            "libphonenumber"
#else
            "not available"
#endif
        "\n"

        "Extractors          : ";

    auto caps = QString::fromLatin1(s);
    ExtractorRepository repo;
    caps += QString::number(repo.extractors().size()) + QLatin1Char('\n');

    return caps;
}
