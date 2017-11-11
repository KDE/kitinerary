#! /bin/sh
$EXTRACT_GRANTLEE_TEMPLATE_STRINGS `find templates -name \*.html` >> html.cpp
$XGETTEXT *.cpp -o $podir/messageviewer_semantic_plugin.pot
rm -f html.cpp
