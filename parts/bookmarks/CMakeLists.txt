SET(kdevbookmarks_SRCS  
bookmarks_part.cpp
bookmarks_widget.cpp
bookmarks_settings.cpp
bookmarks_config.cpp
)

KDE_ADD_UI_FILES(kdevbookmarks_SRCS  bookmarks_settings_base.ui )

KDE_AUTOMOC(${kdevbookmarks_SRCS})

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras  ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

ADD_LIBRARY(kdevbookmarks SHARED ${kdevbookmarks_SRCS})
