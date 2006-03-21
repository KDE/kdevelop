# gnome-common.m4
# 

dnl GNOME_COMMON_INIT

AC_DEFUN([GNOME_COMMON_INIT],
[
	AC_CACHE_VAL(ac_cv_gnome_aclocal_dir,
	[ac_cv_gnome_aclocal_dir="$GNOME_COMMON_MACROS_DIR"])
	AC_CACHE_VAL(ac_cv_gnome_aclocal_flags,
	[ac_cv_gnome_aclocal_flags="$ACLOCAL_FLAGS"])
	GNOME_ACLOCAL_DIR="$ac_cv_gnome_aclocal_dir"
	GNOME_ACLOCAL_FLAGS="$ac_cv_gnome_aclocal_flags"
	AC_SUBST(GNOME_ACLOCAL_DIR)
	AC_SUBST(GNOME_ACLOCAL_FLAGS)

	ACLOCAL="$ACLOCAL $GNOME_ACLOCAL_FLAGS"

	AM_CONDITIONAL(INSIDE_GNOME_DOCU, false)
])

AC_DEFUN([GNOME_DEBUG_CHECK],
[
	AC_ARG_ENABLE(debug, [  --enable-debug turn on debugging [default=no]], enable_debug="$enableval", enable_debug=no)

	if test x$enable_debug = xyes ; then
	    AC_DEFINE(GNOME_ENABLE_DEBUG,1,
		[Enable additional debugging at the expense of performance and size])
	fi
])
