#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` `find . -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/kdevokteta.pot
