#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
set -e
set -x

FREETYPE_VERSION="VER-2-13-2"
POPPLER_VERSION="poppler-25.11.0"
LIBICAL_VERSION="v3.0.17"
LIBXML_VERSION="v2.12.7"
ZXING_VERSION="v2.3.0"
KF_VERSION="v6.20.0"
GEAR_VERSION="release/25.12"

function build_cmake_module() {
    local repo=$1
    shift
    local module=$1
    shift
    local version=$1
    shift

    mkdir -p $BUILD_ROOT
    mkdir -p $STAGING_ROOT
    pushd $BUILD_ROOT

    if ! [ -d $BUILD_ROOT/$module ]; then
        git clone --branch $version --depth 1 $repo $module
    fi
    cd $module

    mkdir build
    cd build
    cmake -DBUILD_TESTING=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_PREFIX_PATH=$STAGING_ROOT \
        -DCMAKE_INSTALL_PREFIX=$STAGING_ROOT \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--as-needed" \
        -DZLIB_USE_STATIC_LIBS=ON \
        -DUSE_DBUS=OFF \
        $@ -DCMAKE_BUILD_TYPE=Release ..

    make -j 4
    make install/fast

    popd
}

function build_kde_module() {
    local module=$1
    shift
    build_cmake_module https://invent.kde.org/$module $module $GEAR_VERSION $@
}

function build_kf_module() {
    local module=$1
    shift
    build_cmake_module https://invent.kde.org/$module $module $KF_VERSION $@
}

build_cmake_module https://gitlab.freedesktop.org/freetype/freetype.git freetype $FREETYPE_VERSION

build_cmake_module https://gitlab.freedesktop.org/poppler/poppler poppler $POPPLER_VERSION \
    -DBUILD_CPP_TESTS=OFF -DBUILD_QT5_TESTS=OFF -DBUILD_QT6_TESTS=OFF -DBUILD_GTK_TESTS=OFF -DENABLE_CPP=OFF \
    -DENABLE_DCTDECODER=unmaintained -DENABLE_GLIB=OFF -DENABLE_GOBJECT_INTROSPECTION=OFF -DENABLE_LIBOPENJPEG=unmaintained \
    -DENABLE_UNSTABLE_API_ABI_HEADERS=ON -DENABLE_UTILS=OFF -DENABLE_NSS3=OFF -DENABLE_LIBTIFF=OFF -DENABLE_LIBCURL=OFF \
    -DENABLE_CMS=none -DWITH_CAIRO=OFF -DWITH_JPEG=OFF -DFONT_CONFIGURATION=generic -DENABLE_BOOST=OFF -DENABLE_QT5=OFF \
    -DENABLE_QT6=OFF -DENABLE_SPLASH=OFF -DENABLE_GPGME=OFF -DENABLE_LCMS=OFF

build_cmake_module https://github.com/libical/libical.git libical $LIBICAL_VERSION \
    -DWITH_CXX_BINDINGS=OFF -DSTATIC_ONLY=ON -DGOBJECT_INTROSPECTION=OFF -DICAL_BUILD_DOCS=OFF \
    -DICAL_GLIB_VAPI=OFF -DICAL_GLIB=OFF -DENABLE_GTK_DOC=OFF -DCMAKE_DISABLE_FIND_PACKAGE_ICU=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_BDB=ON

build_cmake_module https://gitlab.gnome.org/GNOME/libxml2.git libxml2 $LIBXML_VERSION \
    -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_DEBUG=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_ZLIB=OFF

build_cmake_module https://github.com/nu-book/zxing-cpp.git zxing-cpp $ZXING_VERSION \
    -DBUILD_SHARED_LIBRARY=OFF  -DBUILD_EXAMPLES=OFF -DBUILD_BLACKBOX_TESTS=OFF -DBUILD_UNIT_TESTS=OFF -DBUILD_PYTHON_MODULE=OFF

# KDE Frameworks
build_kf_module frameworks/extra-cmake-modules
build_kf_module frameworks/karchive -DWITH_BZIP2=OFF -DWITH_LIBLZMA=OFF -DWITH_LIBZSTD=OFF -DKF_SKIP_PO_PROCESSING=ON
build_kf_module frameworks/kcodecs -DKF_SKIP_PO_PROCESSING=ON
build_kf_module frameworks/kconfig -DKCONFIG_USE_QML=OFF -DKF_SKIP_PO_PROCESSING=ON
build_kf_module frameworks/kcoreaddons -DKCOREADDONS_USE_QML=OFF -DKF_SKIP_PO_PROCESSING=ON
build_kf_module frameworks/ki18n -DBUILD_WITH_QML=OFF -DKI18N_EMBEDDED_ISO_CODES_CACHE=ON -DKF_SKIP_PO_PROCESSING=ON
build_kf_module frameworks/kcalendarcore -DKF_SKIP_PO_PROCESSING=ON
build_kf_module frameworks/kcontacts -DKF_SKIP_PO_PROCESSING=ON

# PIM
build_kde_module pim/kmime -DKF_SKIP_PO_PROCESSING=ON
build_kde_module pim/kpkpass -DKF_SKIP_PO_PROCESSING=ON

export CXXFLAGS="-static-libstdc++ -static-libgcc"
build_kde_module $CI_PROJECT_PATH -DKITINERARY_STANDALONE_CLI_EXTRACTOR=ON -DBUILD_TOOLS=OFF
