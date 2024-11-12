#!/bin/bash
# SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

OPENSSL_VERSION=3.2.3

mkdir -p $BUILD_ROOT
mkdir -p $STAGING_ROOT

pushd $BUILD_ROOT

git clone --branch openssl-$OPENSSL_VERSION --depth 1 https://github.com/openssl/openssl.git
cd openssl

./config --prefix=$STAGING_ROOT --openssldir=$STAGING_ROOT
make -j 4
make install_dev
rm -f $STAGING_ROOT/lib/libcrypto.so*
rm -f $STAGING_ROOT/lib64/libcrypto.so*
rm -f $STAGING_ROOT/lib/libssl.so*
rm -f $STAGING_ROOT/lib64/libssl.so*

popd
