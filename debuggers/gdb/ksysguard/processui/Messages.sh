#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.cc -o $podir/processui.pot
rm -f rc.cpp
