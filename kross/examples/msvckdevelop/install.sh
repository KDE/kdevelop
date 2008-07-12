#!/bin/sh

cp kdevmsvcmanager.desktop $KDEDIR/share/kde4/services/
mkdir -p KDevMSVCManager/
cp KDevMSVCManager/*.py $KDEDIR/lib/kde4/KDevMSVCManager/
