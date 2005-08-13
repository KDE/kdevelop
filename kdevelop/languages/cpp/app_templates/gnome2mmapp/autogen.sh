#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# generated 2004/4/12 12:12:12 EDT by amp8165@localhost.(none)
# using glademm V2.0.0
# I didn't want to put a copy of 'macros' in every generated package
# so I try to find them at autogen.sh time and copy them here.
# (Normally if you have access to a cvs repository a copy of macros is
# put into your directory at checkout time. E.g. cvs.gnome.org/gnome-common)
if [ ! -e macros ]
then
  GLADE_MACROS=`which glade | sed -e 's-bin/glade-share/glade-'`
  if [ -r $GLADE_MACROS/gnome/gnome.m4 ]
  then
    if cp --dereference /dev/null /dev/zero
    then
      cp -r --dereference $GLADE_MACROS/gnome macros
    else
      cp -r $GLADE_MACROS/gnome macros
    fi
  else
    echo "I can't find glade's gnome m4 macros. Please copy them to ./macros and retry."
    exit 2
  fi
fi

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="%{APPNAMELC}"

(test -f $srcdir/configure.in \
## put other tests here
) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

export ACLOCAL_FLAGS="-I `pwd`/macros $ACLOCAL_FLAGS"
. $srcdir/macros/autogen.sh
