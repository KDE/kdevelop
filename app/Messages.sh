#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >>rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/kdevelop.pot
$XGETTEXT -L java `find . -name \*.qml` -j -o $podir/kdevelop.pot
rm -f rc.cpp
