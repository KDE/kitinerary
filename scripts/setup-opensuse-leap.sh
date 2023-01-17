#!/bin/bash
# SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

GCC_VERSION=10
CMAKE_VERSION=3.25.1

# install build dependencies
zypper --non-interactive install \
    curl \
    gzip \
    tar \
    gcc$GCC_VERSION \
    gcc$GCC_VERSION-c++ \
    gettext-runtime \
    gettext-tools \
    git \
    glibc-devel-static \
    gperf \
    make \
    shared-mime-info \
    which \
    zlib-devel-static \
    python3

# gcc-$GCC_VERSION is a non-standard compiler on this platform, hide that
ln -s /usr/bin/gcc-$GCC_VERSION /usr/bin/gcc
ln -s /usr/bin/g++-$GCC_VERSION /usr/bin/g++
ln -s /usr/bin/gcc-ar-$GCC_VERSION /usr/bin/gcc-ar
ln -s /usr/bin/gcc-nm-$GCC_VERSION /usr/bin/gcc-nm
ln -s /usr/bin/gcc-ranlib-$GCC_VERSION /usr/bin/gcc-ranlib
ln -s /usr/bin/gcov-$GCC_VERSION /usr/bin/gcov
ln -s /usr/bin/gcov-dump-$GCC_VERSION /usr/bin/gcov-dump
ln -s /usr/bin/gcov-tool-$GCC_VERSION /usr/bin/gcov-tool
ln -s /usr/bin/cpp-$GCC_VERSION /usr/bin/cpp
ln -s /usr/bin/gcc /usr/bin/cc
ln -s /usr/bin/g++ /usr/bin/c++

# get latest CMake
CMAKE_MINOR_VERSION=`echo $CMAKE_VERSION | sed -e 's/\.[^\.]*$//'`
curl -Lo /tmp/cmake.sh https://cmake.org/files/v$CMAKE_MINOR_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.sh
bash /tmp/cmake.sh --skip-license --prefix=/usr --exclude-subdir
