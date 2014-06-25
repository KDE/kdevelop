set( outofprojectincludesanddefines_SRCS
        outofprojectcustomincludepaths.cpp
        outofprojectincludepathsmanager.cpp
   )

kde4_add_ui_files( outofprojectincludesanddefines_SRCS
        outofprojectcustomincludepaths.ui )

kde4_add_library(kdev4outofprojectincludesanddefines
        ${outofprojectincludesanddefines_SRCS}
    )