SET(kdevclassview_SRCS 
classviewpart.cpp
classviewwidget.cpp
digraphview.cpp
hierarchydlg.cpp
navigator.cpp
viewcombos.cpp
)

KDE_AUTOMOC(${kdevclassview_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras  ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

ADD_LIBRARY(kdevclassview SHARED ${kdevclassview_SRCS})
