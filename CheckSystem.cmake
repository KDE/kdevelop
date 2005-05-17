
#check for libz using the cmake supplied FindZLIB.cmake
INCLUDE(FindZLIB)

IF(ZLIB_FOUND)
   SET(HAVE_LIBZ 1)
ELSE(ZLIB_FOUND)
   SET(HAVE_LIBZ 0)
ENDIF(ZLIB_FOUND)

#check for png
INCLUDE(FindPNG)
IF(PNG_FOUND)
   SET(HAVE_LIBPNG 1)
ELSE(PNG_FOUND)
   SET(HAVE_LIBPNG 0)
ENDIF(PNG_FOUND)

#now check for dlfcn.h using the cmake supplied CHECK_INCLUDE_FILE() macro

CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)

SET(CONFIG_QT_DOCDIR "\"/usr/lib/qt/doc/html/\"")
SET(CONFIG_KDELIBS_DOXYDIR "\"/tmp/\"")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h)
