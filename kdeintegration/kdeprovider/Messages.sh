#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT  *.cpp -o $podir/kdevkdeprovider.pot
rm -f rc.cpp
