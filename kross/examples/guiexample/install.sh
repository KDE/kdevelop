#!/bin/sh

cp kdevguiexample.desktop $KDEDIR/share/kde4/services/
mkdir -p $KDEDIR/share/lib/kde4/GUIExample/
cp GUIExample/*.py $KDEDIR/share/lib/kde4/GUIExample/
