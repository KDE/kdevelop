#!/bin/sh

# Inspired by Makefile.common from coolo
# this script is used to update the .po files

# To update the translations, you will need a specific gettext 
# patched for kde and a lot of patience, tenacity, luck, time ..


# I guess one should only update the .po files when all .cpp files 
# are generated (after a make or scons)

# If you have a better way to do this, do not keep that info 
# for yourself and help me to improve this script, thanks
# (tnagyemail-mail tat yahoo d0tt fr)

SRCDIR=../test1-kconfigxt # srcdir is the directory containing the source code
TIPSDIR=$SRCDIR # tipsdir is the directory containing the tips

KDEDIR=`kde-config --prefix`
EXTRACTRC=extractrc
KDEPOT=`kde-config --prefix`/include/kde.pot
XGETTEXT="xgettext -C -ki18n -ktr2i18n -kI18N_NOOP -ktranslate -kaliasLocale -x $KDEPOT "

## check that kde.pot is available
if ! test -e $KDEPOT; then
	echo "$KDEPOT does not exist, there is something wrong with your installation!"
	XGETTEXT="xgettext -C -ki18n -ktr2i18n -kI18N_NOOP -ktranslate -kaliasLocale "
fi

> rc.cpp

## extract the strings
echo "extracting the strings"

# process the .ui and .rc files
$EXTRACTRC `find $SRCDIR -iname *.rc` >> rc.cpp
$EXTRACTRC `find $SRCDIR -iname *.ui` >> rc.cpp
echo -e 'i18n("_: NAME OF TRANSLATORS\\n"\n"Your names")\ni18n("_: EMAIL OF TRANSLATORS\\n"\n"Your emails")' > $SRCDIR/_translatorinfo.cpp

# process the tips - $SRCDIR is supposed to be where the tips are living
pushd $TIPSDIR; preparetips >tips.cpp; popd

$XGETTEXT `find $SRCDIR -name "*.cpp"` -o kdissert.pot

# remove the intermediate files
rm -f $TIPSDIR/tips.cpp
rm -f rc.cpp
rm -f $SRCDIR/_translatorinfo.cpp

## now merge the .po files ..
echo "merging the .po files"

for i in `ls *.po`; do
    msgmerge $i kdissert.pot -o $i || exit 1
done

## finished
echo "Done"

