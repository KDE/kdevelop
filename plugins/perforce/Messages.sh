#!/bin/sh
RC_FILES=`find . -name \*.rc` `find . -name \*.ui`
[ -n "${RC_FILES}" ] && $EXTRACTRC ${RC_FILES} >> rc.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/kdevperforce.pot
rm -f rc.cpp
