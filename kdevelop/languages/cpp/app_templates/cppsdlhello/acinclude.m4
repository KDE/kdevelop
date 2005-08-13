# Configure paths for SDLmm
# David Hedbor, 2000-07-22
# stolen from SDL 
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SDLMM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDLmm, and define SDLMM_CXXFLAGS and SDLMM_LIBS
dnl
AC_DEFUN(AM_PATH_SDLMM,
[dnl 
dnl Get the cflags and libraries from the ismap-config script
dnl
AC_ARG_WITH(sdlmm-prefix,[  --with-sdlmm-prefix=PFX   Prefix where SDLmm is installed (optional)],
            sdlmm_prefix="$withval", sdlmm_prefix="")
AC_ARG_WITH(sdlmm-exec-prefix,[  --with-sdlmm-exec-prefix=PFX Exec prefix where SDLmm is installed (optional)],
            sdlmm_exec_prefix="$withval", sdlmm_exec_prefix="")
AC_ARG_ENABLE(sdlmmtest, [  --disable-sdlmmtest       Do not try to compile and run a test SDLmm program],
		    , enable_sdlmmtest=yes)

  if test x$sdlmm_exec_prefix != x ; then
     sdlmm_args="$sdlmm_args --exec-prefix=$sdlmm_exec_prefix"
     if test x${SDLMM_CONFIG+set} != xset ; then
        SDLMM_CONFIG=$sdlmm_exec_prefix/bin/sdlmm-config
     fi
  fi
  if test x$sdlmm_prefix != x ; then
     sdlmm_args="$sdlmm_args --prefix=$sdlmm_prefix"
     if test x${SDLMM_CONFIG+set} != xset ; then
        SDLMM_CONFIG=$sdlmm_prefix/bin/sdlmm-config
     fi
  fi

  AC_PATH_PROG(SDLMM_CONFIG, sdlmm-config, no)
  min_sdlmm_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDLmm version >= $min_sdlmm_version)
  no_sdlmm=""
  if test "$SDLMM_CONFIG" = "no" ; then
    no_sdlmm=yes
  else
    SDLMM_CXXFLAGS=`$SDLMM_CONFIG $sdlmmconf_args --cflags`
    SDLMM_LIBS=`$SDLMM_CONFIG $sdlmmconf_args --libs`

    sdlmm_major_version=`$SDLMM_CONFIG $sdlmm_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdlmm_minor_version=`$SDLMM_CONFIG $sdlmm_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdlmm_micro_version=`$SDLMM_CONFIG $sdlmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdlmmtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $SDLMM_CXXFLAGS"
      LIBS="$LIBS $SDLMM_LIBS"
dnl
dnl Now check if the installed SDLmm is sufficiently new. (Also sanity
dnl checks the results of sdlmm-config to some extent
dnl
      rm -f conf.sdlmmtest
      AC_TRY_RUN([
#include <cstdio>
#include <cstring>
#include "SDLmm/sdlmm.h"
char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((std::strlen (str) + 1) * sizeof(char));
      std::strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdlmmtest");
  */
  { FILE *fp = fopen("conf.sdlmmtest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdlmm_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdlmm_version");
     return 1;
   }

   if (($sdlmm_major_version > major) ||
      (($sdlmm_major_version == major) && ($sdlmm_minor_version > minor)) ||
      (($sdlmm_major_version == major) && ($sdlmm_minor_version == minor) && ($sdlmm_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdlmm-config --version' returned %d.%d.%d, but the minimum version\n", $sdlmm_major_version, $sdlmm_minor_version, $sdlmm_micro_version);
      printf("*** of SDLmm required is %d.%d.%d. If sdlmm-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdlmm-config was wrong, set the environment variable SDLMM_CONFIG\n");
      printf("*** to point to the correct copy of sdlmm-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_sdlmm=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdlmm" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SDLMM_CONFIG" = "no" ; then
       echo "*** The sdlmm-config script installed by SDLmm could not be found"
       echo "*** If SDLmm was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDLMM_CONFIG environment variable to the"
       echo "*** full path to sdlmm-config."
     else
       if test -f conf.sdlmmtest ; then
        :
       else
          echo "*** Could not run SDLmm test program, checking why..."
          CXXFLAGS="$CXXFLAGS $SDLMM_CXXFLAGS"
          LIBS="$LIBS $SDLMM_LIBS"
          AC_TRY_LINK([
#include <cstdio>
#include "sdlmm.h"
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDLmm or finding the wrong"
          echo "*** version of SDLmm. If it is not finding SDLmm, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SDLmm was incorrectly installed"
          echo "*** or that you have moved SDLmm since it was installed. In the latter case, you"
          echo "*** may want to edit the sdlmm-config script: $SDLMM_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDLMM_CXXFLAGS=""
     SDLMM_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDLMM_CXXFLAGS)
  AC_SUBST(SDLMM_LIBS)
  rm -f conf.sdlmmtest
])
