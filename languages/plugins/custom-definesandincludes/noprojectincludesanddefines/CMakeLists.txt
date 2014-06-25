set( noprojectincludesanddefines_SRCS
        noprojectcustomincludepaths.cpp
        noprojectincludepathsmanager.cpp
   )

kde4_add_ui_files( noprojectincludesanddefines_SRCS
        noprojectcustomincludepaths.ui )

kde4_add_library(kdev4noprojectincludesanddefines STATIC
        ${noprojectincludesanddefines_SRCS}
    )

set_target_properties(kdev4noprojectincludesanddefines PROPERTIES POSITION_INDEPENDENT_CODE ON)