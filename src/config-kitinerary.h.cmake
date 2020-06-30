/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

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

#cmakedefine HAVE_KCAL
#cmakedefine HAVE_LIBXML2
#cmakedefine HAVE_PHONENUMBER
#cmakedefine HAVE_OPENSSL_RSA

#define CMAKE_INSTALL_FULL_LIBEXECDIR_KF5 "${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}"

#endif
