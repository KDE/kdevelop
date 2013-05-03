#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT  *.cpp -o $podir/kdevexecuteplasmoid.pot
rm -f rc.cpp
