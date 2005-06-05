#this file defines the following variables:
#KDE_INCLUDE_DIR
#KDE_LIB_DIR
#KDE_SERVICETYPES_DIR
#DCOPIDL
#DCOPIDL2CPP
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.1)

SET(QT_MT_REQUIRED TRUE)
#SET(QT_MIN_VERSION "3.0.0")

#this line includes FindQt.cmake, which searches the Qt library and headers
INCLUDE(FindQt)

#add the definitions found by FindQt to the current definitions
ADD_DEFINITIONS(${QT_DEFINITIONS})

SET(QT_AND_KDECORE_LIBS ${QT_LIBRARIES} kdecore)

#add some KDE specific stuff
ADD_DEFINITIONS(-Wnon-virtual-dtor -Wno-long-long -Wundef -ansi -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -O2 -Wformat-security -Wmissing-format-attribute -fno-exceptions -fno-check-new -fno-common)
#only on linux:
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  ADD_DEFINITIONS(-D_XOPEN_SOURCE=500 -D_BSD_SOURCE)
ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")


#SET(CMAKE_SHARED_LINKER_FLAGS "-avoid-version -module -Wl,--no-undefined -Wl,--allow-shlib-undefined")
SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")
SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")

#now try to find some kde stuff

#at first the KDE include direcory
FIND_PATH(KDE_INCLUDE_DIR kurl.h
  $ENV{KDEDIR}/include
  /opt/kde/include
  /opt/kde3/include
  /usr/local/include
  /usr/include/
  /usr/include/kde
  /usr/local/include/kde
)

#now the KDE library directory
FIND_PATH(KDE_LIB_DIR libkdecore.so
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
  /usr/lib
  /usr/local/lib
)

#now the KDE service types directory
FIND_PATH(KDE_SERVICETYPES_DIR ktexteditor.desktop
  $ENV{KDEDIR}/share/servicetypes/
  /opt/kde/share/servicetypes/
  /opt/kde3/share/servicetypes/
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

#bail out if something wasn't found
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


#add the found Qt and KDE include directories to the current include path
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR} ${KDE_INCLUDE_DIR} .)

LINK_DIRECTORIES(${KDE_LIB_DIR})

SET(CMAKE_VERBOSE_MAKEFILE ON)
