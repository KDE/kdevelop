#!/bin/sh
$XGETTEXT `find duchain codecompletion navigation -name \*.qml -o -name \*.cpp -o -name \*.h` -o $podir/kdevqmljssupport.pot
