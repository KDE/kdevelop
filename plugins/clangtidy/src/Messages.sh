#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/kdevclangtidy.pot
rm -f rc.cpp
