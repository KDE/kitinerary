#!/bin/bash
# SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

CMAKE_VERSION=3.27.9

# patch repository URLs to use vault.centos.org
# WARNING: this is using EOL software, we should not do this!
function patchYumRepositories() {
    for f in /etc/yum.repos.d/*.repo; do
        sed -i "s/^mirrorlist/#mirrorlist/" $f
        sed -i "s,#baseurl=http://mirror.centos,baseurl=http://vault.centos," $f
        sed -i "s,# baseurl=http://mirror.centos,baseurl=http://vault.centos," $f
    done
}

# install build dependencies
patchYumRepositories
yum install -y centos-release-scl
patchYumRepositories
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
    perl-core \
    perl-IPC-Cmd \
    perl-Test-Simple \
    perl-Text-Template

# get latest CMake
CMAKE_MINOR_VERSION=`echo $CMAKE_VERSION | sed -e 's/\.[^\.]*$//'`
curl -Lo /tmp/cmake.sh https://cmake.org/files/v$CMAKE_MINOR_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.sh
bash /tmp/cmake.sh --skip-license --prefix=/usr --exclude-subdir
