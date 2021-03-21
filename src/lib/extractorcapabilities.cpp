/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include "extractorcapabilities.h"
#include "extractor.h"

#include <KItinerary/ExtractorRepository>

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

        "PDF support         : "
#ifdef HAVE_POPPLER
              "poppler (" KPOPPLER_VERSION_STRING ")"
#else
              "not available"
#endif
        "\n"

        "iCal support        : "
#ifdef HAVE_KCAL
            "kcal"
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

        "RSA support         : "
#ifdef HAVE_OPENSSL_RSA
            "openssl"
#else
            "not available"
#endif
        "\n"

        "Extractor scripts   : ";

    auto caps = QString::fromLatin1(s);
    ExtractorRepository repo;
    caps += QString::number(repo.allExtractors().size()) + QLatin1Char('\n');

    return caps;
}
