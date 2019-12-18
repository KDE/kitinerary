/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include "extractorcapabilities.h"
#include "extractor.h"
#include "extractorrepository.h"

#include <QString>

using namespace KItinerary;

QString ExtractorCapabilities::capabilitiesString()
{
    const char s[] =
        "Engine version      : " KITINERARY_VERSION_STRING "\n"

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
            "zxing"
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
