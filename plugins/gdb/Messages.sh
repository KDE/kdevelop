#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >>rc.cpp
find . \( -name \*.cc -o -name \*.cpp -o -name \*.h \) -print0 | xargs -0 $XGETTEXT -o $podir/kdevgdb.pot
rm -f rc.cpp
