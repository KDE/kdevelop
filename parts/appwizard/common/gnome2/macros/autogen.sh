#!/bin/sh
# Run this to generate all the initial makefiles, etc.

#name of package
PKG_NAME=${PKG_NAME:-Package}
srcdir=${srcdir:-.}

# default version requirements ...
REQUIRED_AUTOCONF_VERSION=${REQUIRED_AUTOCONF_VERSION:-2.53}
REQUIRED_AUTOMAKE_VERSION=${REQUIRED_AUTOMAKE_VERSION:-1.4}
REQUIRED_LIBTOOL_VERSION=${REQUIRED_LIBTOOL_VERSION:-1.4.3}
REQUIRED_GETTEXT_VERSION=${REQUIRED_GETTEXT_VERSION:-0.10.40}
REQUIRED_GLIB_GETTEXT_VERSION=${REQUIRED_GLIB_GETTEXT_VERSION:-2.2.0}
REQUIRED_INTLTOOL_VERSION=${REQUIRED_INTLTOOL_VERSION:-0.25}
REQUIRED_PKG_CONFIG_VERSION=${REQUIRED_PKG_CONFIG_VERSION:-0.14.0}
REQUIRED_GTK_DOC_VERSION=${REQUIRED_GTK_DOC_VERSION:-1.0}
REQUIRED_DOC_COMMON_VERSION=${REQUIRED_DOC_COMMON_VERSION:-2.3.0}

# a list of required m4 macros.  Package can set an initial value
REQUIRED_M4MACROS=${REQUIRED_M4MACROS:-}
FORBIDDEN_M4MACROS=${FORBIDDEN_M4MACROS:-}

# if GNOME2_DIR set, modify ACLOCAL_FLAGS ...
if [ -n "$GNOME2_DIR" ]; then
    ACLOCAL_FLAGS="-I $GNOME2_DIR/share/aclocal $ACLOCAL_FLAGS"
    LD_LIBRARY_PATH="$GNOME2_DIR/lib:$LD_LIBRARY_PATH"
    PATH="$GNOME2_DIR/bin:$PATH"
    export PATH
    export LD_LIBRARY_PATH
fi


# Not all echo versions allow -n, so we check what is possible. This test is
# based on the one in autoconf.
case `echo "testing\c"; echo 1,2,3`,`echo -n testing; echo 1,2,3` in
  *c*,-n*) ECHO_N= ;;
  *c*,*  ) ECHO_N=-n ;;
  *)       ECHO_N= ;;
esac

# some terminal codes ...
boldface="`tput bold 2>/dev/null`"
normal="`tput sgr0 2>/dev/null`"
printbold() {
    echo $ECHO_N "$boldface"
    echo "$@"
    echo $ECHO_N "$normal"
}    
printerr() {
    echo "$@" >&2
}

# Usage:
#     compare_versions MIN_VERSION ACTUAL_VERSION
# returns true if ACTUAL_VERSION >= MIN_VERSION
compare_versions() {
    ch_min_version=$1
    ch_actual_version=$2
    ch_status=0
    IFS="${IFS=         }"; ch_save_IFS="$IFS"; IFS="."
    set $ch_actual_version
    for min in $ch_min_version; do
        ch_cur=`echo $1 | sed 's/[^0-9].*$//'`; shift # remove letter suffixes
        if [ -z "$ch_min" ]; then break; fi
        if [ -z "$ch_cur" ]; then ch_status=1; break; fi
        if [ $ch_cur -gt $ch_min ]; then break; fi
        if [ $ch_cur -lt $ch_min ]; then ch_status=1; break; fi
    done
    IFS="$ch_save_IFS"
    return $ch_status
}

# Usage:
#     version_check PACKAGE VARIABLE CHECKPROGS MIN_VERSION SOURCE
# checks to see if the package is available
version_check() {
    vc_package=$1
    vc_variable=$2
    vc_checkprogs=$3
    vc_min_version=$4
    vc_source=$5
    vc_status=1

    vc_checkprog=`eval echo "\\$$vc_variable"`
    if [ -n "$vc_checkprog" ]; then
	printbold "using $vc_checkprog for $vc_package"
	return 0
    fi

    printbold "checking for $vc_package >= $vc_min_version..."
    for vc_checkprog in $vc_checkprogs; do
	echo $ECHO_N "  testing $vc_checkprog... "
	if $vc_checkprog --version < /dev/null > /dev/null 2>&1; then
	    vc_actual_version=`$vc_checkprog --version | head -1 | \
                               sed 's/^.*[ 	]\([0-9.]*[a-z]*\).*$/\1/'`
	    if compare_versions $vc_min_version $vc_actual_version; then
		echo "found $vc_actual_version"
		# set variable
		eval "$vc_variable=$vc_checkprog"
		vc_status=0
		break
	    else
		echo "too old (found version $vc_actual_version)"
	    fi
	else
	    echo "not found."
	fi
    done
    if [ "$vc_status" != 0 ]; then
	printerr "***Error***: You must have $vc_package >= $vc_min_version installed"
	printerr "  to build $PKG_NAME.  Download the appropriate package for"
	printerr "  from your distribution or get the source tarball at"
        printerr "    $vc_source"
	printerr
    fi
    return $vc_status
}

# Usage:
#     require_m4macro filename.m4
# adds filename.m4 to the list of required macros
require_m4macro() {
    case "$REQUIRED_M4MACROS" in
	$1\ * | *\ $1\ * | *\ $1) ;;
	*) REQUIRED_M4MACROS="$REQUIRED_M4MACROS $1" ;;
    esac
}

forbid_m4macro() {
    case "$FORBIDDEN_M4MACROS" in
	$1\ * | *\ $1\ * | *\ $1) ;;
	*) FORBIDDEN_M4MACROS="$FORBIDDEN_M4MACROS $1" ;;
    esac
}

# Usage:
#     check_m4macros
# Checks that all the requested macro files are in the aclocal macro path
# Uses REQUIRED_M4MACROS and ACLOCAL variables.
check_m4macros() {
    # construct list of macro directories
    cm_macrodirs="`$ACLOCAL --print-ac-dir`"
    set - $ACLOCAL_FLAGS
    while [ $# -gt 0 ]; do
	if [ "$1" = "-I" ]; then
	    cm_macrodirs="$cm_macrodirs $2"
	    shift
	fi
	shift
    done

    cm_status=0
    if [ -n "$REQUIRED_M4MACROS" ]; then
	printbold "Checking for required M4 macros..."
	# check that each macro file is in one of the macro dirs
	for cm_macro in $REQUIRED_M4MACROS; do
	    cm_macrofound=false
	    for cm_dir in $cm_macrodirs; do
		if [ -f "$cm_dir/$cm_macro" ]; then
		    cm_macrofound=true
		    break
		fi
		# The macro dir in Cygwin environments may contain a file
		# called dirlist containing other directories to look in.
		if [ -f "$cm_dir/dirlist" ]; then
		    for cm_otherdir in `cat $cm_dir/dirlist`; do
			if [ -f "$cm_otherdir/$cm_macro" ]; then
			    cm_macrofound=true
		            break
			fi
		    done
		fi
	    done
	    if $cm_macrofound; then
		:
	    else
		printerr "  $cm_macro not found"
		cm_status=1
	    fi
	done
    fi
    if [ -n "$FORBIDDEN_M4MACROS" ]; then
	printbold "Checking for forbidden M4 macros..."
	# check that each macro file is in one of the macro dirs
	for cm_macro in $FORBIDDEN_M4MACROS; do
	    cm_macrofound=false
	    for cm_dir in $cm_macrodirs; do
		if [ -f "$cm_dir/$cm_macro" ]; then
		    cm_macrofound=true
		    break
		fi
	    done
	    if $cm_macrofound; then
		printerr "  $cm_macro found (should be cleared from macros dir)"
		cm_status=1
	    fi
	done
    fi
    if [ "$cm_status" != 0 ]; then
	printerr "***Error***: some autoconf macros required to build $PKG_NAME"
	printerr "  were not found in your aclocal path, or some forbidden"
	printerr "  macros were found.  Perhaps you need to adjust your"
	printerr "  ACLOCAL_PATH?"
	printerr
    fi
    return $cm_status
}

# try to catch the case where the macros2/ directory hasn't been cleared out.
forbid_m4macro gnome-cxx-check.m4

want_libtool=false
want_gettext=false
want_glib_gettext=false
want_intltool=false
want_pkg_config=false
want_gtk_doc=false

configure_files="`find $srcdir -name '{arch}' -prune -o -name configure.ac -print -or -name configure.in -print`"
for configure_ac in $configure_files; do
    if grep "^A[CM]_PROG_LIBTOOL" $configure_ac >/dev/null; then
	want_libtool=true
    fi
    if grep "^AM_GNU_GETTEXT" $configure_ac >/dev/null; then
	want_gettext=true
    fi
    if grep "^AM_GLIB_GNU_GETTEXT" $configure_ac >/dev/null; then
	want_glib_gettext=true
    fi
    if grep "^AC_PROG_INTLTOOL" $configure_ac >/dev/null; then
	want_intltool=true
    fi
    if grep "^PKG_CHECK_MODULES" $configure_ac >/dev/null; then
	want_pkg_config=true
    fi
    if grep "^GTK_DOC_CHECK" $configure_ac >/dev/null; then
	want_gtk_doc=true
    fi
done

DIE=0

#tell Mandrake autoconf wrapper we want autoconf 2.5x, not 2.13
WANT_AUTOCONF_2_5=1
export WANT_AUTOCONF_2_5
version_check autoconf AUTOCONF 'autoconf2.50 autoconf autoconf-2.53' $REQUIRED_AUTOCONF_VERSION \
    "http://ftp.gnu.org/pub/gnu/autoconf/autoconf-$REQUIRED_AUTOCONF_VERSION.tar.gz" || DIE=1
AUTOHEADER=`echo $AUTOCONF | sed s/autoconf/autoheader/`

case $REQUIRED_AUTOMAKE_VERSION in
    1.4*) automake_progs="automake-1.4" ;;
    1.5*) automake_progs="automake-1.7 automake-1.6 automake-1.5" ;;
    1.6*) automake_progs="automake-1.7 automake-1.6" ;;
    1.7*) automake_progs="automake-1.7" ;;
    1.8*) automake_progs="automake-1.8" ;;
esac
version_check automake AUTOMAKE "$automake_progs automake" $REQUIRED_AUTOMAKE_VERSION \
    "http://ftp.gnu.org/pub/gnu/automake/automake-$REQUIRED_AUTOMAKE_VERSION.tar.gz" || DIE=1
ACLOCAL=`echo $AUTOMAKE | sed s/automake/aclocal/`

if $want_libtool; then
    version_check libtool LIBTOOLIZE libtoolize $REQUIRED_LIBTOOL_VERSION \
        "http://ftp.gnu.org/pub/gnu/libtool/libtool-$REQUIRED_LIBTOOL_VERSION.tar.gz" || DIE=1
    require_m4macro libtool.m4
fi

if $want_gettext; then
    version_check gettext GETTEXTIZE gettextize $REQUIRED_GETTEXT_VERSION \
        "http://ftp.gnu.org/pub/gnu/gettext/gettext-$REQUIRED_GETTEXT_VERSION.tar.gz" || DIE=1
    require_m4macro gettext.m4
fi

if $want_glib_gettext; then
    version_check glib-gettext GLIB_GETTEXTIZE glib-gettextize $REQUIRED_GLIB_GETTEXT_VERSION \
        "ftp://ftp.gtk.org/pub/gtk/v2.2/glib-$REQUIRED_GLIB_GETTEXT_VERSION.tar.gz" || DIE=1
    require_m4macro glib-gettext.m4
fi

if $want_intltool; then
    version_check intltool INTLTOOLIZE intltoolize $REQUIRED_INTLTOOL_VERSION \
        "http://ftp.gnome.org/pub/GNOME/sources/intltool/" || DIE=1
    require_m4macro intltool.m4
fi

if $want_pkg_config; then
    version_check pkg-config PKG_CONFIG pkg-config $REQUIRED_PKG_CONFIG_VERSION \
        "'http://www.freedesktop.org/software/pkgconfig/releases/pkgconfig-$REQUIRED_PKG_CONFIG_VERSION.tar.gz" || DIE=1
    require_m4macro pkg.m4
fi

if $want_gtk_doc; then
    version_check gtk-doc GTKDOCIZE gtkdocize $REQUIRED_GTK_DOC_VERSION \
        "http://ftp.gnome.org/pub/GNOME/sources/gtk-doc/" || DIE=1
    require_m4macro gtk-doc.m4
fi

if [ "x$USE_COMMON_DOC_BUILD" = "xyes" ]; then
    version_check gnome-common DOC_COMMON gnome-doc-common \
        $REQUIRED_DOC_COMMON_VERSION " " || DIE=1
fi

check_m4macros || DIE=1

if [ "$DIE" -eq 1 ]; then
  exit 1
fi

if test -z "$*"; then
  printerr "**Warning**: I am going to run \`configure' with no arguments."
  printerr "If you wish to pass any to it, please specify them on the"
  printerr \`$0\'" command line."
  printerr
fi

topdir=`pwd`
for configure_ac in $configure_files; do 
    dirname=`dirname $configure_ac`
    basename=`basename $configure_ac`
    if test -f $dirname/NO-AUTO-GEN; then
	echo skipping $dirname -- flagged as no auto-gen
    else
	printbold "Processing $configure_ac"
	cd $dirname

	aclocalinclude="$ACLOCAL_FLAGS"
	printbold "Running $ACLOCAL..."
	$ACLOCAL $aclocalinclude # || exit 1

	if grep "GNOME_AUTOGEN_OBSOLETE" aclocal.m4 >/dev/null; then
	    printerr "*** obsolete gnome macros were used in $configure_ac"
	fi

	if grep "^A[CM]_PROG_LIBTOOL" $basename >/dev/null; then
	    printbold "Running $LIBTOOLIZE..."
	    $LIBTOOLIZE --force || exit 1
	fi
	if grep "^AM_GLIB_GNU_GETTEXT" $basename >/dev/null; then
	    printbold "Running $GLIB_GETTEXTIZE... Ignore non-fatal messages."
	    echo "no" | $GLIB_GETTEXTIZE --force --copy || exit 1
	elif grep "^AM_GNU_GETTEXT" $basename >/dev/null; then
	   if grep "^AM_GNU_GETTEXT_VERSION" $basename > /dev/null; then
	   	printbold "Running autopoint..."
		autopoint --force || exit 1
	   else
	    	printbold "Running $GETTEXTIZE... Ignore non-fatal messages."
	    	echo "no" | $GETTEXTIZE --force --copy || exit 1
	   fi
	fi
	if grep "^AC_PROG_INTLTOOL" $basename >/dev/null; then
	    printbold "Running $INTLTOOLIZE..."
	    $INTLTOOLIZE --force --automake || exit 1
	fi
	if grep "^GTK_DOC_CHECK" $basename >/dev/null; then
	    printbold "Running $GTKDOCIZE..."
	    $GTKDOCIZE || exit 1
	fi
	if grep "^A[CM]_CONFIG_HEADER" $basename >/dev/null; then
	    printbold "Running $AUTOHEADER..."
	    $AUTOHEADER || exit 1
	fi
	if [ "x$USE_COMMON_DOC_BUILD" = "xyes" ]; then
	    printbold "Running gnome-doc-common..."
	    gnome-doc-common --copy || exit 1
	fi

	printbold "Running $AUTOMAKE..."
	$AUTOMAKE --gnu --add-missing || exit 1

	printbold "Running $AUTOCONF..."
	$AUTOCONF || exit 1

	cd $topdir
    fi
done

conf_flags="--enable-maintainer-mode"

#if test x$NOCONFIGURE = x; then
#    printbold Running $srcdir/configure $conf_flags "$@" ...
#    $srcdir/configure $conf_flags "$@" \
#	&& echo Now type \`make\' to compile $PKG_NAME || exit 1
#else
#    echo Skipping configure process.
#fi
