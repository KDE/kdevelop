#ifndef _LIB_COMPAT_KDEV_KSTDACTION_H_
#define _LIB_COMPAT_KDEV_KSTDACTION_H_

#if !defined(KDE_IS_VERSION)
// for kdelibs < 3.1.0
# define pasteText paste
#else
# if !KDE_IS_VERSION(3,1,93)
// for kdelibs < 3.1.93
#  define pasteText paste
# endif
#endif

#endif