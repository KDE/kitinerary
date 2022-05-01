/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include "extractorcapabilities.h"

#include <KItinerary/ExtractorRepository>

#ifdef HAVE_KCAL
#include <kcalendarcore_version.h>
#endif

#include <QString>

using namespace KItinerary;

QString ExtractorCapabilities::capabilitiesString()
{
    const char s[] =
        "Engine version      : " KITINERARY_VERSION_STRING "\n"
        "Qt version          : " QT_VERSION_STR "\n"

        "HTML support        : "
#ifdef HAVE_LIBXML2
              "libxml2"
#else
              "not available"
#endif
        "\n"

        "PDF support         : poppler (" KPOPPLER_VERSION_STRING ")\n"

        "iCal support        : "
#ifdef HAVE_KCAL
            "kcal (" KCALENDARCORE_VERSION_STRING ")"
#else
            "not available"
#endif
        "\n"

        "Barcode decoder     : "
#ifdef HAVE_ZXING
            "zxing (" ZXING_VERSION_STRING ")"
#else
            "not available"
#endif
        "\n"

        "Phone number decoder: "
#ifdef HAVE_PHONENUMBER
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
