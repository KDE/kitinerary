#!/bin/bash
# SPDX-FileCopyrightText: Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
set -e
set -x

DEPLOY_ROOT=$BUILD_ROOT/$CI_PROJECT_PATH/build/

mkdir -p $DEPLOY_ROOT/share/
cp -rv $STAGING_ROOT/share/locale $DEPLOY_ROOT/share/

# remove catalogs we don't use
for i in iso_15924.mo iso_3166.mo iso_3166_2.mo iso_3166-3.mo iso_4217.mo 'iso_639*.mo' karchive5_qt.qm kcodecs5_qt.qm kconfig5_qt.qm kcoreaddons5_qt.qm libkmime5.mo kcontacts5.mo ki18n5.mo LC_SCRIPTS; do
    find $DEPLOY_ROOT -name $i | xargs rm -rf
done

# remove languages we have no own translation for
for i in `ls $DEPLOY_ROOT/share/locale`; do
    if ! [ -f $DEPLOY_ROOT/share/locale/$i/LC_MESSAGES/kitinerary6.mo ]; then
        echo "Dropping language $i"
        rm -rf $DEPLOY_ROOT/share/locale/$i
    fi
done

# strip binary
strip $DEPLOY_ROOT/bin/kitinerary-extractor

# determing version
# VERSION=$CI_COMMIT_REF_SLUG
VERSION=`cat $BUILD_ROOT/$CI_PROJECT_PATH/build/version.txt`

# prepare output for publishing
PACKAGE_ROOT=$CI_PROJECT_DIR/kde-ci-packages/
mkdir -p $PACKAGE_ROOT

OUTPUT_FILE=$PACKAGE_ROOT/kitinerary-extractor-x86_64-$VERSION.tgz
tar cvz -C $DEPLOY_ROOT -f $OUTPUT_FILE bin/kitinerary-extractor share
sha256sum $OUTPUT_FILE | head -c 66 > $OUTPUT_FILE.sha256
