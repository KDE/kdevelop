PROJECT(kdevelop)

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/KDE.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/KDEMacros.cmake)

SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib/interfaces ${CMAKE_CURRENT_SOURCE_DIR}/lib/util)

SUBDIRS( bdb lib parts)

#SUBDIRS( bdb lib parts languages buildtools editors vcs src pics doc mimetypes embedded kdevdesigner)
