SET(kdevastyle_SRCS 
astyle_part.cpp
astyle_widget.cpp
astyle_adaptor.cpp
)

KDE_ADD_UI_FILES(kdevastyle_SRCS astyleconfig.ui )

KDE_AUTOMOC(${kdevastyle_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/astyle)

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras  ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

ADD_LIBRARY(kdevastyle SHARED ${kdevastyle_SRCS})
