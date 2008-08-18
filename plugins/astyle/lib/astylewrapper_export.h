#ifndef ASTYLEWRAPPER_EXPORT_H
#define ASTYLEWRAPPER_EXPORT_H
 
// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>
 
#ifndef ASTYLEWRAPPER_EXPORT
# if defined(MAKE_ASTYLEWRAPPER_LIB)
   // We are building this library
#  define ASTYLEWRAPPER_EXPORT KDE_EXPORT
# else
   // We are using this library
#  define ASTYLEWRAPPER_EXPORT KDE_IMPORT
# endif
#endif
 
# ifndef ASTYLEWRAPPER_EXPORT_DEPRECATED
#  define ASTYLEWRAPPER_EXPORT_DEPRECATED KDE_DEPRECATED ASTYLEWRAPPER_EXPORT
# endif
 
#endif