#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >>rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -L java `find . -name \*.qml` -o $podir/kdevelop.pot
rm -f rc.cpp
