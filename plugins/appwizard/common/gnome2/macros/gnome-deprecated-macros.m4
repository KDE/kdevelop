-*- mode: autoconf -*-

# GNOME_AUTOGEN_OBSOLETE
# this marker is checked for in the aclocal.m4 file to check for bad macros ...

AU_DEFUN([AM_GNOME2_GETTEXT], [AM_GLIB_GNU_GETTEXT])
AU_DEFUN([GNOME_GTKDOC_CHECK], [GTK_DOC_CHECK])
AU_DEFUN([GNOME2_X_CHECKS], [])
AU_DEFUN([GNOME_PTHREAD_CHECK], [])
AU_DEFUN([GNOME_CHECK_PKGCONFIG], [])
AU_DEFUN([GNOME_REQUIRE_PKGCONFIG], [])
AU_DEFUN([GNOME_PLATFORM_GNOME_2], [])

AU_DEFUN([GNOME_CHECK_CXX],
[
  # see if a C++ compiler exists and works
  AC_PROG_CXX
  if test "x$ac_cv_prog_cxx_works" = xno; then
    AC_MSG_WARN(ifelse([$1], , "No C++ compiler", [$1]))
  fi
  AM_CONDITIONAL(CXX_PRESENT, test "x$ac_cv_prog_cxx_works" != xno)
])

# for aclocal-1.4's benefit
#   AC_DEFUN([AM_GNOME2_GETTEXT], [])
#   AC_DEFUN([GNOME_GTKDOC_CHECK], [])
#   AC_DEFUN([GNOME2_X_CHECKS], [])
#   AC_DEFUN([GNOME_PTHREAD_CHECK], [])
#   AC_DEFUN([GNOME_CHECK_PKGCONFIG], [])
#   AC_DEFUN([GNOME_REQUIRE_PKGCONFIG], [])
#   AC_DEFUN([GNOME_PLATFORM_GNOME_2], [])
#   AC_DEFUN([GNOME_CHECK_CXX], [])
