#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui | grep -v '/tests/'` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevclangtidy.pot
rm -f rc.cpp
