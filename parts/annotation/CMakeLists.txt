SET(kdevannotation_SRCS 
annotationpart.cpp
annotationwidget.cpp
annotationglobalconfig.cpp
annotationprojectconfig.cpp
annotationitem.cpp
)

KDE_ADD_UI_FILES(kdevannotation_SRCS  annotationglobalconfigbase.ui annotationprojectconfigbase.ui )

KDE_AUTOMOC(${kdevannotation_SRCS})

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras  ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

ADD_LIBRARY(kdevannotation SHARED ${kdevannotation_SRCS})
