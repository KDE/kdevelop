
SET(QT_MT_REQUIRED TRUE)
#SET(QT_MIN_VERSION "3.0.0")

INCLUDE(FindQt)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-Wnon-virtual-dtor -Wno-long-long -Wundef -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -O2 -Wformat-security -Wmissing-format-attribute -fno-exceptions -fno-check-new -fno-common)


FIND_PATH(KDE_INCLUDE_DIR kurl.h
  $ENV{KDEDIR}/include
  /opt/kde/include
  /opt/kde3/include
  /usr/local/include
  /usr/include/
  /usr/include/kde
  /usr/local/include/kde
)

FIND_PATH(KDE_LIB_DIR libkdecore.so
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
)

#now search for the dcop utilities
FIND_PROGRAM(DCOPIDL NAME dcopidl PATHS 
  $ENV{KDEDIR}/bin
  /opt/kde/bin
  /opt/kde3/bin)

FIND_PROGRAM(DCOPIDL2CPP NAME dcopidl2cpp PATHS 
  $ENV{KDEDIR}/bin
  /opt/kde/bin
  /opt/kde3/bin)

IF(NOT KDE_INCLUDE_DIR)
   MESSAGE(FATAL_ERROR "Didn't find KDE headers")
ENDIF(NOT KDE_INCLUDE_DIR)

IF(NOT KDE_LIB_DIR)
   MESSAGE(FATAL_ERROR "Didn't find KDE core library")
ENDIF(NOT KDE_LIB_DIR)

IF(NOT DCOPIDL2CPP)
   MESSAGE(FATAL_ERROR "Didn't find the dcopidl2cpp preprocessor")
ENDIF(NOT DCOPIDL2CPP)

IF(NOT DCOPIDL)
   MESSAGE(FATAL_ERROR "Didn't find the dcopidl preprocessor")
ENDIF(NOT DCOPIDL)


INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} ${KDE_INCLUDE_DIR})

LINK_DIRECTORIES(${KDE_LIB_DIR})

SET(CMAKE_VERBOSE_MAKEFILE ON)
