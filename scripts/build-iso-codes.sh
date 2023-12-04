#!/bin/bash
# SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

mkdir -p $BUILD_ROOT
mkdir -p $STAGING_ROOT

pushd $BUILD_ROOT

git clone --depth 1 https://salsa.debian.org/iso-codes-team/iso-codes.git
cd iso-codes

./configure --prefix=$STAGING_ROOT
make -j 4
make install

popd
