/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
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

#define CMAKE_INSTALL_FULL_LIBEXECDIR_KF5 "${CMAKE_INSTALL_FULL_LIBEXECDIR_KF5}"

#endif
