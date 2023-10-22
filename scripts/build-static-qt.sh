#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
set -e
set -x

function build-static-qt-module() {
    local module=$1
    shift

    mkdir -p $BUILD_ROOT
    mkdir -p $STAGING_ROOT
    pushd $BUILD_ROOT

    git clone --branch kde/5.15 --depth 1 https://invent.kde.org/qt/qt/$module
    cd $module
    mkdir build
    cd build
    if [ $module == "qtbase" ]; then
        ../configure -prefix $STAGING_ROOT $@
    else
        $STAGING_ROOT/bin/qmake .. $@
    fi
    make -j 4
    make install

    popd
}

build-static-qt-module qtbase \
    -release -optimize-size \
    -qpa offscreen \
    -no-pch \
    -no-icu \
    -no-dbus \
    -no-glib \
    -no-xcb -no-opengl -no-feature-vulkan \
    -no-feature-sql \
    -no-widgets \
    -no-feature-network \
    -no-feature-sha3-fast \
    -no-accessibility \
    -no-feature-animation -no-feature-clipboard -no-feature-colornames -no-feature-cursor -no-feature-cssparser -no-feature-draganddrop \
    -no-feature-gestures -no-feature-im \
    -no-feature-image_heuristic_mask -no-feature-image_text -no-feature-imageformat_bmp -no-feature-imageformat_ppm -no-feature-imageformat_xbm -no-feature-imageformat_xpm -no-feature-imageformatplugin -no-feature-movie \
    -no-feature-picture -no-feature-pdf \
    -no-feature-concurrent -no-feature-future -no-feature-sharedmemory -no-feature-systemsemaphore \
    -no-feature-statemachine \
    -no-feature-syntaxhighlighter \
    -no-feature-tabletevent -no-feature-wheelevent \
    -no-feature-texthtmlparser -no-feature-textodfwriter \
    -no-feature-topleveldomain \
    -no-feature-validator \
    -no-feature-desktopservices \
    -no-feature-proxymodel -no-feature-stringlistmodel \
    -no-feature-testlib \
    -no-fontconfig -no-harfbuzz \
    -no-feature-sessionmanager \
    -no-feature-textmarkdownreader -no-feature-textmarkdownwriter \
    -static -confirm-license -opensource -make libs -make tools

build-static-qt-module qtdeclarative -- \
    -no-feature-qml-debug \
    -no-feature-qml-preview \
    -no-feature-qml-profiler \
    -no-feature-qml-animation \
    -no-feature-qml-list-model \
    -no-feature-qml-worker-script

build-static-qt-module qttools

# Patch .prl files to use static zlib
for i in `find $STAGING_ROOT -name "*.prl"`; do
    sed -i -e 's,-lz,/usr/lib64/libz.a,g' $i
done
