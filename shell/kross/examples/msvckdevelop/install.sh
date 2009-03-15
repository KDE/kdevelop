#!/bin/sh

cp kdevmsvcmanager.desktop $KDEDIR/share/kde4/services/
mkdir -p $KDEDIR/lib/kde4/KDevMSVCManager/
cp KDevMSVCManager/*.py $KDEDIR/lib/kde4/KDevMSVCManager/
