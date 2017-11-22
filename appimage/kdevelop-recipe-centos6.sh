#!/bin/bash

# Halt on errors
set -e

# Be verbose
set -x

# Now we are inside CentOS 6
grep -r "CentOS release 6" /etc/redhat-release || exit 1

git_pull_rebase_helper()
{
    git fetch
    git stash || true
    git rebase $(git rev-parse --abbrev-ref --symbolic-full-name @{u}) || true
    git stash pop || true
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

QTVERSION=5.9.2
QTVERSION_SHORT=5.9
QTDIR=/usr/local/Qt-${QTVERSION}/

if [ -z "$KDEVELOP_VERSION" ]; then
    KDEVELOP_VERSION=5.2
fi
if [ -z "$KDEV_PG_QT_VERSION" ]; then
    KDEV_PG_QT_VERSION=2.0
fi
KF5_VERSION=v5.37.0
KDE_PLASMA_VERSION=master # need libksysguard master (contains a0e69617442d720c76da5ebe3323e7a977929db4 -- patch which makes plasma dep optional)
KDE_APPLICATION_VERSION=v16.12.3
GRANTLEE_VERSION=v5.1.0

export LLVM_ROOT=/opt/llvm/
export PATH=/opt/rh/python27/root/usr/bin/:$PATH
export LD_LIBRARY_PATH=/opt/rh/python27/root/usr/lib64:$LD_LIBRARY_PATH

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment. That's
# not always set correctly in CentOS 6.7
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# Determine which architecture should be built
if [[ "$(arch)" = "i686" || "$(arch)" = "x86_64" ]] ; then
  ARCH=$(arch)
else
  echo "Architecture could not be determined"
  exit 1
fi

# Make sure we build from the /, parts of this script depends on that. We also need to run as root...
cd  /

# Build AppImageKit
#rm -Rf /AppImageKit
if [ ! -d AppImageKit ] ; then
  git clone  --depth 1 https://github.com/probonopd/AppImageKit.git /AppImageKit
fi

cd /AppImageKit/
git checkout stable/v1.0
git_pull_rebase_helper
git reset --hard
./build.sh
cd /

# Use the new compiler
. /opt/rh/devtoolset-4/enable

# TODO: Use these vars more
export FAKEROOT=/kdevelop.appdir
export PREFIX=/kdevelop.appdir/usr/
export SRC=$HOME/src/
export BUILD=$HOME/build
export CMAKE_PREFIX_PATH=$QTDIR:/kdevelop.appdir/share/llvm/

# if the library path doesn't point to our usr/lib, linking will be broken and we won't find all deps either
export LD_LIBRARY_PATH=/usr/lib64/:/usr/lib:/kdevelop.appdir/usr/lib:$QTDIR/lib/:/opt/python3.6/lib/:$LD_LIBRARY_PATH

# Workaround for: On CentOS 6, .pc files in /usr/lib/pkgconfig are not recognized
# However, this is where .pc files get installed when bulding libraries... (FIXME)
# I found this by comparing the output of librevenge's "make install" command
# between Ubuntu and CentOS 6
ln -sf /usr/share/pkgconfig /usr/lib/pkgconfig

# Prepare the install location
if [ -z "$SKIP_PRUNE" ]; then
    rm -rf /kdevelop.appdir/ || true
    mkdir -p /kdevelop.appdir/usr

    # refresh ldconfig cache
    ldconfig

    # make sure lib and lib64 are the same thing
    mkdir -p /kdevelop.appdir/usr/lib
    cd  /kdevelop.appdir/usr
    ln -s lib lib64
fi

# start building the deps
function build_project
{ (
    PROJECT=$1
    VERSION=$2

    # clone if not there
    mkdir -p $SRC
    cd $SRC
    if ( test -d $PROJECT )
    then
        echo "$PROJECT already cloned"
        cd $PROJECT
        git stash
        git reset --hard
        git fetch
        git fetch --tags
        cd ..
    else
        if [ -z "$CUSTOM_GIT_URL" ]; then
            git clone git://anongit.kde.org/$PROJECT
        else
            git clone $CUSTOM_GIT_URL
        fi
    fi

    cd $PROJECT
    git checkout $VERSION
    git rebase $(git rev-parse --abbrev-ref --symbolic-full-name @{u}) || true # git rebase will fail if a tag is checked out
    git stash pop || true
    cd ..

    if [ "$PROJECT" = "knotifications" ]; then
    cd $PROJECT
        echo "patching knotifications"
    git reset --hard
    cat > no_phonon.patch << EOF
diff --git a/CMakeLists.txt b/CMakeLists.txt
index b97425f..8f15f08 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -59,10 +59,10 @@ find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
 find_package(KF5Codecs ${KF5_DEP_VERSION} REQUIRED)
 find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
 
-find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)
+find_package(Phonon4Qt5 4.6.60 NO_MODULE)
 set_package_properties(Phonon4Qt5 PROPERTIES
    DESCRIPTION "Qt-based audio library"
-   TYPE REQUIRED
+   TYPE OPTIONAL
    PURPOSE "Required to build audio notification support")
 if (Phonon4Qt5_FOUND)
   add_definitions(-DHAVE_PHONON4QT5)
EOF
    cat no_phonon.patch |patch -p1
    cd ..
    fi

    # create build dir
    mkdir -p $BUILD/$PROJECT

    # go there
    cd $BUILD/$PROJECT

    # cmake it
    cmake3 $SRC/$PROJECT -G Ninja -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX $3

    # make
    ninja

    # install
    ninja install
) }

function build_framework
{ (
    build_project $1 $KF5_VERSION $2
) }

# KDE Frameworks
if [ -z "$SKIP_FRAMEWORKS" ]; then
build_framework extra-cmake-modules

build_framework kconfig
build_framework kguiaddons
build_framework ki18n
build_framework kitemviews
build_framework sonnet
build_framework kwindowsystem
build_framework kwidgetsaddons
build_framework kcompletion
build_framework kdbusaddons
build_framework karchive
build_framework kcoreaddons
build_framework kjobwidgets
build_framework kcrash
build_framework kservice
build_framework kcodecs
build_framework kauth
build_framework kconfigwidgets
build_framework kiconthemes
build_framework ktextwidgets
build_framework kglobalaccel
build_framework kxmlgui
build_framework kbookmarks
build_framework solid
build_framework kio
build_framework kparts
build_framework kitemmodels
build_framework threadweaver
build_framework attica
build_framework knewstuff
build_framework syntax-highlighting
build_framework ktexteditor
build_framework kpackage
build_framework kdeclarative
build_framework kcmutils
build_framework knotifications
build_framework knotifyconfig
build_framework kdoctools
build_framework breeze-icons -DBINARY_ICONS_RESOURCE=1
build_framework kpty
build_framework kinit 
fi

# KDE Plasma
build_project libksysguard $KDE_PLASMA_VERSION
build_project kdecoration $KDE_PLASMA_VERSION # for breeze
build_project breeze $KDE_PLASMA_VERSION

# KDE Applications
build_project libkomparediff2 $KDE_APPLICATION_VERSION
build_project kate $KDE_APPLICATION_VERSION # for snippet plugin, see T3826
build_project konsole $KDE_APPLICATION_VERSION

# Extra
(CUSTOM_GIT_URL=https://github.com/steveire/grantlee.git build_project grantlee $GRANTLEE_VERSION)

# KDevelop
build_project kdevelop-pg-qt $KDEV_PG_QT_VERSION
build_project kdevelop $KDEVELOP_VERSION
build_project kdev-php $KDEVELOP_VERSION

# Build kdev-python
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH/kdevelop.appdir/usr/lib/
build_project kdev-python $KDEVELOP_VERSION

# Install some colorschemes
cd $SRC
$SCRIPT_DIR/install_colorschemes.sh

cd /kdevelop.appdir

# FIXME: How to find out which subset of plugins is really needed? I used strace when running the binary
mkdir -p ./usr/lib/qt5/plugins/

if [ -e $(dirname $QTDIR/plugins/bearer) ] ; then
  PLUGINS=$(dirname $QTDIR/plugins/bearer)
else
  PLUGINS=../../$QTVERSION_SHORT/gc*/plugins/
fi
echo $PLUGINS # /usr/lib64/qt5/plugins if build system Qt is found
cp -r $PLUGINS/bearer ./usr/lib/qt5/plugins/
cp -r $PLUGINS/generic ./usr/lib/qt5/plugins/
cp -r $PLUGINS/imageformats ./usr/lib/qt5/plugins/
cp -r $PLUGINS/platforms ./usr/lib/qt5/plugins/
cp -r $PLUGINS/iconengines ./usr/lib/qt5/plugins/
cp -r $PLUGINS/platforminputcontexts ./usr/lib/qt5/plugins/
# cp -r $PLUGINS/platformthemes ./usr/lib/qt5/plugins/
cp -r $PLUGINS/xcbglintegrations ./usr/lib/qt5/plugins/

cp -R /kdevelop.appdir/usr/lib/grantlee/ /kdevelop.appdir/usr/lib/qt5/plugins/
rm -Rf /kdevelop.appdir/usr/lib/grantlee

cp -ru /usr/share/mime/* /kdevelop.appdir/usr/share/mime
update-mime-database /kdevelop.appdir/usr/share/mime/

cp -R ./usr/lib/plugins/* ./usr/lib/qt5/plugins/
rm -Rf ./usr/lib/plugins/

cp $(ldconfig -p | grep libsasl2.so.2 | cut -d ">" -f 2 | xargs) ./usr/lib/
# Fedora 23 seemed to be missing SOMETHING from the Centos 6.7. The only message was:
# This application failed to start because it could not find or load the Qt platform plugin "xcb".
# Setting export QT_DEBUG_PLUGINS=1 revealed the cause.
# QLibraryPrivate::loadPlugin failed on "/usr/lib64/qt5/plugins/platforms/libqxcb.so" : 
# "Cannot load library /usr/lib64/qt5/plugins/platforms/libqxcb.so: (/lib64/libEGL.so.1: undefined symbol: drmGetNodeTypeFromFd)"
# Which means that we have to copy libEGL.so.1 in too
cp $(ldconfig -p | grep libEGL.so.1 | cut -d ">" -f 2 | xargs) ./usr/lib/ # Otherwise F23 cannot load the Qt platform plugin "xcb"
cp $(ldconfig -p | grep libxcb.so.1 | cut -d ">" -f 2 | xargs) ./usr/lib/ 
cp $(ldconfig -p | grep libfreetype.so.6 | cut -d ">" -f 2 | xargs) ./usr/lib/ # For Fedora 20

ldd usr/bin/kdevelop | grep "=>" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./usr/lib || true
cp /usr/bin/cmake usr/bin/cmake
ldd usr/bin/cmake | grep "=>" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./usr/lib || true
#ldd usr/lib64/kdevelop/*.so  | grep "=>" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./usr/lib || true
#ldd usr/lib64/plugins/imageformats/*.so  | grep "=>" | awk '{print $3}' | xargs -I '{}' cp -v '{}' ./usr/lib || true

ldd usr/lib/qt5/plugins/platforms/libqxcb.so | grep "=>" | awk '{print $3}'  |  xargs -I '{}' cp -v '{}' ./usr/lib || true

# Copy in the indirect dependencies
FILES=$(find . -type f -executable)

for FILE in $FILES ; do
    echo "*** Processing:" $FILE
    ldd "${FILE}" | grep "=>" | awk '{print $3}' | xargs -I '{}' cp -vu '{}' usr/lib || true
done

# The following are assumed to be part of the base system
rm -f usr/lib/libcom_err.so.2 || true
rm -f usr/lib/libcrypt.so.1 || true
rm -f usr/lib/libdl.so.2 || true
rm -f usr/lib/libexpat.so.1 || true
#rm -f usr/lib/libfontconfig.so.1 || true
rm -f usr/lib/libgcc_s.so.1 || true
rm -f usr/lib/libglib-2.0.so.0 || true
rm -f usr/lib/libgpg-error.so.0 || true
rm -f usr/lib/libgssapi_krb5.so.2 || true
rm -f usr/lib/libgssapi.so.3 || true
rm -f usr/lib/libhcrypto.so.4 || true
rm -f usr/lib/libheimbase.so.1 || true
rm -f usr/lib/libheimntlm.so.0 || true
rm -f usr/lib/libhx509.so.5 || true
rm -f usr/lib/libICE.so.6 || true
rm -f usr/lib/libidn.so.11 || true
rm -f usr/lib/libk5crypto.so.3 || true
rm -f usr/lib/libkeyutils.so.1 || true
rm -f usr/lib/libkrb5.so.26 || true
rm -f usr/lib/libkrb5.so.3 || true
rm -f usr/lib/libkrb5support.so.0 || true
# rm -f usr/lib/liblber-2.4.so.2 || true # needed for debian wheezy
# rm -f usr/lib/libldap_r-2.4.so.2 || true # needed for debian wheezy
rm -f usr/lib/libm.so.6 || true
rm -f usr/lib/libp11-kit.so.0 || true
rm -f usr/lib/libpcre.so.3 || true
rm -f usr/lib/libpthread.so.0 || true
rm -f usr/lib/libresolv.so.2 || true
rm -f usr/lib/libroken.so.18 || true
rm -f usr/lib/librt.so.1 || true
rm -f usr/lib/libSM.so.6 || true
rm -f usr/lib/libusb-1.0.so.0 || true
rm -f usr/lib/libuuid.so.1 || true
rm -f usr/lib/libwind.so.0 || true

# Remove these libraries, we need to use the system versions; this means 11.04 is not supported (12.04 is our baseline)
rm -f usr/lib/libGL.so.* || true
rm -f usr/lib/libdrm.so.* || true

#rm -f usr/lib/libz.so.1 || true

# These seem to be available on most systems but not Ubuntu 11.04
# rm -f usr/lib/libffi.so.6 usr/lib/libGL.so.1 usr/lib/libglapi.so.0 usr/lib/libxcb.so.1 usr/lib/libxcb-glx.so.0 || true

# Delete potentially dangerous libraries
rm -f usr/lib/libstdc* usr/lib/libgobject* usr/lib/libc.so.* || true
# Do NOT delete libX* because otherwise on Ubuntu 11.04:
# loaded library "Xcursor" malloc.c:3096: sYSMALLOc: Assertion (...) Aborted

# We don't bundle the developer stuff
rm -rf usr/include || true
rm -rf usr/lib/cmake || true
rm -rf usr/lib/pkgconfig || true
rm -rf usr/share/ECM/ || true
rm -rf usr/share/gettext || true
rm -rf usr/share/pkgconfig || true

strip -g $(find usr) || true

# We do not bundle this, so let's not search that inside the AppImage. 
# Fixes "Qt: Failed to create XKB context!" and lets us enter text
#sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/lib/qt5/plugins/platforminputcontexts/libcomposeplatforminputcontextplugin.so
#sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/lib/libQt5XcbQpa.so.5

# Workaround for:
# D-Bus library appears to be incorrectly set up;
# failed to read machine uuid: Failed to open
# The file is more commonly in /etc/machine-id
# sed -i -e 's|/var/lib/dbus/machine-id|//././././etc/machine-id|g' ./usr/lib/libdbus-1.so.3
# or
rm -f ./usr/lib/libdbus-1.so.3 || true

# Remove python
rm -f ./usr/bin/python*
rm -f ./usr/bin/pydoc*
rm -f ./usr/bin/pyenv*

# remove big execs
rm -f ./usr/bin/verify-uselistorder
rm -f ./usr/bin/obj2yaml ./usr/bin/yaml2obj
rm -f ./usr/bin/kwrite ./usr/bin/kate

cp /kdevelop.appdir/usr/lib/libexec/kf5/* /kdevelop.appdir/usr/bin/

cd /
if [ ! -d appimage-exec-wrapper ]; then
    git clone git://anongit.kde.org/scratch/brauch/appimage-exec-wrapper
fi;
cd /appimage-exec-wrapper/
make clean
make

cd /kdevelop.appdir
cp -v /appimage-exec-wrapper/exec.so exec_wrapper.so

cat > AppRun << EOF
#!/bin/bash

DIR="\`dirname \"\$0\"\`" 
DIR="\`( cd \"\$DIR\" && pwd )\`"
export APPDIR=\$DIR

export LD_PRELOAD=\$DIR/exec_wrapper.so

export APPIMAGE_ORIGINAL_QML2_IMPORT_PATH=\$QML2_IMPORT_PATH
export APPIMAGE_ORIGINAL_LD_LIBRARY_PATH=\$LD_LIBRARY_PATH
export APPIMAGE_ORIGINAL_QT_PLUGIN_PATH=\$QT_PLUGIN_PATH
export APPIMAGE_ORIGINAL_XDG_DATA_DIRS=\$XDG_DATA_DIRS
export APPIMAGE_ORIGINAL_PATH=\$PATH
export APPIMAGE_ORIGINAL_PYTHONHOME=\$PYTHONHOME

export QML2_IMPORT_PATH=\$DIR/usr/lib/qml:\$QML2_IMPORT_PATH
export LD_LIBRARY_PATH=\$DIR/usr/lib/:\$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=\$DIR/usr/lib/qt5/plugins/
export XDG_DATA_DIRS=\$DIR/usr/share/:\$XDG_DATA_DIRS
export PATH=\$DIR/usr/bin:\$PATH
export KDE_FORK_SLAVES=1
export PYTHONHOME=\$DIR/usr/

export APPIMAGE_STARTUP_QML2_IMPORT_PATH=\$QML2_IMPORT_PATH
export APPIMAGE_STARTUP_LD_LIBRARY_PATH=\$LD_LIBRARY_PATH
export APPIMAGE_STARTUP_QT_PLUGIN_PATH=\$QT_PLUGIN_PATH
export APPIMAGE_STARTUP_XDG_DATA_DIRS=\$XDG_DATA_DIRS
export APPIMAGE_STARTUP_PATH=\$PATH
export APPIMAGE_STARTUP_PYTHONHOME=\$PYTHONHOME

export KDEV_DISABLE_PLUGINS=KDevWelcomePage

cd \$HOME

kdevelop \$@
EOF
chmod +x AppRun

cat > kdevelop.desktop << EOF
[Desktop Entry]
GenericName=Integrated development environment
Name=KDevelop
MimeType=text/plain;
Exec=AppRun -b %U
StartupNotify=true
X-KDE-HasTempFileOption=true
Icon=kdevelop
X-DocPath=kdevelop/index.html
Type=Application
Terminal=false
InitialPreference=9
Categories=Qt;KDE;Utility;TextEditor;
EOF

cp $SRC/kdevelop/app/icons/48-apps-kdevelop.png kdevelop.png
cp -R /usr/lib/python3.6 /kdevelop.appdir/usr/lib/
rm -Rf /kdevelop.appdir/usr/lib/python3.6/{test,config-3.5m,__pycache__,site-packages,lib-dynload,distutils,idlelib,unittest,tkinter,ensurepip}

mkdir -p /kdevelop.appdir/usr/share/kdevelop/
cp $BUILD/breeze-icons/icons/breeze-icons.rcc /kdevelop.appdir/usr/share/kdevelop/icontheme.rcc
rm -Rf /kdevelop.appdir/usr/share/icons/breeze* # not needed because of the rcc
rm -f /kdevelop.appdir/usr/bin/llvm*
rm -f /kdevelop.appdir/usr/bin/clang*
rm -f /kdevelop.appdir/usr/bin/opt
rm -f /kdevelop.appdir/usr/bin/lli
rm -f /kdevelop.appdir/usr/bin/sancov
rm -f /kdevelop.appdir/usr/bin/cmake
rm -f /kdevelop.appdir/usr/bin/python
rm -Rf /kdevelop.appdir/usr/lib/pkgconfig
rm -Rf /kdevelop.appdir/usr/share/man
rm -Rf /kdevelop.appdir/usr/share/locale
rm -Rf /kdevelop.appdir/usr/lib/libLTO.so

#At first it seems like "we shouldn't ship X11", but actually we should; the X11 protocol is sort of guaranteed to stay compatible,
#while these libraries are not.
# rm -Rf /kdevelop.appdir/usr/lib/libxcb*
# add that back in
# cp /usr/lib64/libxcb-keysyms.so.1 /kdevelop.appdir/usr/lib/
# rm -Rf /kdevelop.appdir/usr/lib/{libX11.so.6,libXau.so.6,libXext.so.6,libXi.so.6,libXxf86vm.so.1,libX11-xcb.so.1,libXdamage.so.1,libXfixes.so.3,libXrender.so.1}

rm -f /kdevelop.appdir/usr/bin/llc
rm -f /kdevelop.appdir/usr/bin/bugpoint

find /kdevelop.appdir -name '*.a' -exec rm {} \;

cd /

APP=KDevelop

VERSION="git"

if [[ "$ARCH" = "x86_64" ]] ; then
    APPIMAGE=$APP"-"$VERSION"-x86_64.AppImage"
fi
if [[ "$ARCH" = "i686" ]] ; then
    APPIMAGE=$APP"-"$VERSION"-i386.AppImage"
fi
echo $APPIMAGE

mkdir -p /out

rm -f /out/*.AppImage || true
AppImageKit/AppImageAssistant.AppDir/package /kdevelop.appdir/ /out/$APPIMAGE

chmod a+rwx /out/$APPIMAGE # So that we can edit the AppImage outside of the Docker container
