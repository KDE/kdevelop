#!/bin/sh
$XGETTEXT `find . -name \*.qml` --language JavaScript --kde -o $podir/kdevwelcomepage.pot
rm -f rc.cpp
