#!/bin/bash
# SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

mkdir -p $BUILD_ROOT
mkdir -p $STAGING_ROOT

pushd $BUILD_ROOT

git clone --branch OpenSSL_1_1_1-stable --depth 1 https://github.com/openssl/openssl.git
cd openssl

./config --prefix=$STAGING_ROOT --openssldir=$STAGING_ROOT
make -j 4
make install_dev
rm -f $STAGING_ROOT/lib/libcrypto.so*
rm -f $STAGING_ROOT/lib64/libcrypto.so*
rm -f $STAGING_ROOT/lib/libssl.so*
rm -f $STAGING_ROOT/lib64/libssl.so*

popd
