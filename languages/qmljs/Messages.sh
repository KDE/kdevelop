#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >>rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevqmljs.pot
rm -f rc.cpp
