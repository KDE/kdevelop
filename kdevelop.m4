AC_DEFUN(KDEV_CHECK_QTDOC,
[
AC_MSG_CHECKING([for Qt documentation])

ac_qt_docdirs=""
AC_ARG_WITH(qt_docdirs, 
[  --with-qtdocdir=DIR   where the Qt documentation is installed ],
ac_qt_docdirs=""
qt_docdirs=""
[
  ac_qt_docdirs=$withval
],
)

qt_docdirs="/usr/doc/qt-doc/html /usr/local/qt/html /usr/local/lib/qt/html /usr/lib/qt/doc/html /usr/X11/lib/qt/html /usr/X11/lib/qt/doc/html" 
qt_docdirs="$ac_qtdocdirs $qt_docdirs"
AC_FIND_FILE(classes.html, $qt_docdirs, qt_docdir)
AC_MSG_RESULT($qt_docdir)
if test "$qt_docdir" = NO; then
  qt_docdir=""
fi
AC_DEFINE_UNQUOTED(QT_DOCDIR, "$qt_docdir")
]) 


AC_DEFUN(KDEV_CHECK_KDELIBSDOC,
[
AC_MSG_CHECKING([for kdelibs documentation])

ac_kdelibs_docdirs=""
AC_ARG_WITH(kdelibs_docdirs, 
[  --with-kdelibsdocdir=DIR   where the kdelibs documentation is installed ],
ac_kdelibs_docdirs=""
kdelibs_docdirs=""
[
  ac_kdelibs_docdirs=$withval
],
)

kdelibs_docdirs="/usr/doc/kdelibs-doc/html" 
kdelibs_docdirs="$ac_qtdocdirs $kdelibs_docdirs"
AC_FIND_FILE(kdecore/index.html, $kdelibs_docdirs, kdelibs_docdir)
AC_MSG_RESULT($kdelibs_docdir)
if test "$kdelibs_docdir" = NO; then
  kdelibs_docdir=""
fi
AC_DEFINE_UNQUOTED(KDELIBS_DOCDIR, "$kdelibs_docdir")
]) 
