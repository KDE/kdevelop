# - Try to find ThreadWeaver
# Once done this will define
#
#  THREADWEAVER_FOUND - system has threadweaver
#  THREADWEAVER_INCLUDE_DIR - the ThreadWeaver include directory
#  THREADWEAVER_LIBRARIES - the libraries needed to use ThreadWeaver
#  THREADWEAVER_DEFINITIONS - Compiler switches required for using ThreadWeaver

if (THREADWEAVER_INCLUDE_DIR AND THREADWEAVER_LIBRARIES)

    # in cache already
    SET(THREADWEAVER_FOUND TRUE)

else (THREADWEAVER_INCLUDE_DIR AND THREADWEAVER_LIBRARIES)

    FIND_PATH(THREADWEAVER_INCLUDE_DIR ThreadWeaver.h
      ${KDE4_INCLUDE_DIR}/threadweaver
    )

    FIND_LIBRARY(THREADWEAVER_LIBRARIES ThreadWeaver
      ${KDE4_LIB_DIR}
    )

    if (THREADWEAVER_INCLUDE_DIR AND THREADWEAVER_LIBRARIES)
       set(THREADWEAVER_FOUND TRUE)
    endif (THREADWEAVER_INCLUDE_DIR AND THREADWEAVER_LIBRARIES)

    if (THREADWEAVER_FOUND)
      if (NOT ThreadWeaver_FIND_QUIETLY)
        message(STATUS "Found ThreadWeaver: ${THREADWEAVER_LIBRARIES}")
      endif (NOT ThreadWeaver_FIND_QUIETLY)
    else (THREADWEAVER_FOUND)
     if (ThreadWeaver_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find ThreadWeaver, you should install it - it is currently in trunk/kdenonbeta/threadweaver")
     endif (ThreadWeaver_FIND_REQUIRED)
    endif (THREADWEAVER_FOUND)

    MARK_AS_ADVANCED(THREADWEAVER_INCLUDE_DIR THREADWEAVER_LIBRARIES)

endif (THREADWEAVER_INCLUDE_DIR AND THREADWEAVER_LIBRARIES)
