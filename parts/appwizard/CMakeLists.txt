SET(kdevappwizard_SRCS 
appwizardfactory.cpp 
appwizardpart.cpp
appwizarddlg.cpp
importdlg.cpp
filepropspage.cpp 
misc.cpp
kdevfile.cpp 
kdevlicense.cpp 
profilesupport.cpp )

KDE_ADD_UI_FILES(kdevappwizard_SRCS filepropspagebase.ui appwizarddlgbase.ui importdlgbase.ui vcs_form.ui )

KDE_AUTOMOC(${kdevappwizard_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras  ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/widgets)

ADD_LIBRARY(kdevappwizard SHARED ${kdevappwizard_SRCS})
