#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui | grep -v '/tests/'` >> rc.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevclangtidy.pot
rm -f rc.cpp
