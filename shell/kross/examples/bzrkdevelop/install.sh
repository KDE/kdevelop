#!/bin/sh

cp kdevbzr.desktop $KDEDIR/share/kde4/services/
mkdir -p $KDEDIR/lib/kde4/kdevbzr
cp -r kdevbzr/*.py $KDEDIR/lib/kde4/kdevbzr/
