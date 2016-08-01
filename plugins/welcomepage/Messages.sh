#!/bin/sh
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/kdevwelcomepage.pot
rm -f rc.cpp
