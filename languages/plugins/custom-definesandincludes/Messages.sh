#!/bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >>rc.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevcustomdefinesandincludes.pot
rm -f rc.cpp
