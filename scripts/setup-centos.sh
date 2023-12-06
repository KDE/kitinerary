#!/bin/bash
# SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

CMAKE_VERSION=3.27.9

# install build dependencies
yum install -y centos-release-scl
yum install -y \
    devtoolset-11-gcc-c++ \
    rh-git227 \
    glibc-devel \
    gettext \
    gperf \
    make \
    shared-mime-info \
    which \
    zlib-devel \
    zlib-static \
    python3 \
    perl-IPC-Cmd \
    perl-Test-Simple

# get latest CMake
CMAKE_MINOR_VERSION=`echo $CMAKE_VERSION | sed -e 's/\.[^\.]*$//'`
curl -Lo /tmp/cmake.sh https://cmake.org/files/v$CMAKE_MINOR_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.sh
bash /tmp/cmake.sh --skip-license --prefix=/usr --exclude-subdir
