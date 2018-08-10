#!/bin/sh
$XGETTEXT `find . -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kdevbazaar.pot
