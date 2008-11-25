#! /usr/bin/env bash
for plugin in `find -maxdepth 1 -type d -not -name . -printf "%f\n"` ; do
    $EXTRACTRC `find $plugin -name \*.rc` >> rc.cpp || exit 11
    $EXTRACTRC `find $plugin -name \*.ui` >> rc.cpp || exit 12
    $XGETTEXT -kaliasLocal `find $plugin -name \*.cc -o -name \*.cpp -o -name \*.h` rc.cpp -o $podir/kdev${plugin}.pot
    rm -f rc.cpp
done
