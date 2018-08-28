#!/bin/sh

if [ -z "$PREFIX" ]; then
     PREFIX=/tmp/fakeroot/usr
fi

VER=5.13.4 # Plasma version
COLORSCHEMESDIR=$PREFIX/share/color-schemes
mkdir -p $COLORSCHEMESDIR

wget -nc -O breeze.tar.xz https://download.kde.org/stable/plasma/$VER/breeze-$VER.tar.xz
tar xf breeze.tar.xz
cd breeze-$VER
cp -v colors/* $COLORSCHEMESDIR

wget -nc -O plasma-desktop.tar.xz https://download.kde.org/stable/plasma/$VER/plasma-desktop-$VER.tar.xz
tar xf plasma-desktop.tar.xz
cd plasma-desktop-$VER
cp -v ./kcms/colors/schemes/* $COLORSCHEMESDIR
