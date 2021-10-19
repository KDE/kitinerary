/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <ki18n_version.h> // ### temporary until we depend on >= KF 5.88

#ifndef CONFIG_KITINERARY_H
#define CONFIG_KITINERARY_H

#cmakedefine HAVE_POPPLER
// namespaced by "K" to not interfere with defines poppler provides itself
#define KPOPPLER_VERSION_STRING "@Poppler_VERSION@"
#define KPOPPLER_VERSION_MAJOR @POPPLER_VERSION_MAJOR@
#define KPOPPLER_VERSION_MINOR @POPPLER_VERSION_MINOR@
#define KPOPPLER_VERSION_PATCH @POPPLER_VERSION_PATCH@
#define KPOPPLER_VERSION ((@POPPLER_VERSION_MAJOR@<<16)|(@POPPLER_VERSION_MINOR@<<8)|(@POPPLER_VERSION_PATCH@))

#cmakedefine HAVE_ZXING
#define ZXING_VERSION_STRING "@ZXing_VERSION@"
#define ZXING_VERSION_MAJOR @ZXing_VERSION_MAJOR@
#define ZXING_VERSION_MINOR @ZXing_VERSION_MINOR@
#define ZXING_VERSION_PATCH @ZXing_VERSION_PATCH@
#define ZXING_VERSION ((@ZXing_VERSION_MAJOR@<<16)|(@ZXing_VERSION_MINOR@<<8)|(@ZXing_VERSION_PATCH@))

// QT_VERSION_CHECK isn't available in here for the below check
#define K_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

// this might compile with older versions too, but it only actually works post 1.1.1
#ifdef HAVE_ZXING
#if ZXING_VERSION > K_VERSION_CHECK(1, 1, 1)
#define ZXING_USE_READBARCODE
#endif
#endif

#cmakedefine HAVE_KCAL
#cmakedefine HAVE_LIBXML2
#cmakedefine HAVE_PHONENUMBER

#if KI18N_VERSION >= K_VERSION_CHECK(5, 88, 0)
#define HAVE_KI18N_LOCALE_DATA 1
#else
#define HAVE_KI18N_LOCALE_DATA 0
#endif

#define CMAKE_INSTALL_FULL_LIBEXECDIR_KF5 "${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}"

#endif
