SET(kdevdiff_SRCS 
diffpart.cpp
diffdlg.cpp
diffwidget.cpp
)

KDE_AUTOMOC(${kdevdiff_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions)

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras  ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

ADD_LIBRARY(kdevdiff SHARED ${kdevdiff_SRCS})
