
AC_INIT
AM_INIT_AUTOMAKE(testprog,[0.1])

AC_PROG_MAKE_SET

# python interpreter check
AC_PATH_PROG([PYTHON],[python])
if test -z "${PYTHON}"  ; then
   AC_MSG_ERROR([you need Python to use this program])
fi

# PyQT check
AC_MSG_CHECKING([whether PyQT can be used by Python])
pyqt_present=no
cat >pyqttest.py <<END
import qt
END
${PYTHON} pyqttest.py >/dev/null 2>&1 && pyqt_present=yes
AC_MSG_RESULT([$pyqt_present])

# pyuic check
AC_PATH_PROG([PYUIC],[pyuic])
if test -z "${PYUIC}" ; then
   AC_MSG_ERROR([cannot find pyuic (part of the PyQT package)])
fi
AC_SUBST([PYUIC])
AC_SUBST([PYTHON])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
