set( noprojectincludesanddefines_SRCS
        noprojectcustomincludepaths.cpp
        noprojectincludepathsmanager.cpp
   )

ki18n_wrap_ui( noprojectincludesanddefines_SRCS
        noprojectcustomincludepaths.ui )

add_library(kdev4noprojectincludesanddefines STATIC
        ${noprojectincludesanddefines_SRCS}
    )

target_link_libraries( kdev4noprojectincludesanddefines 
LINK_PRIVATE
    KDev::Util
    KDev::Language
    ${KDE4_THREADWEAVER_LIBRARIES}
)

set_target_properties(kdev4noprojectincludesanddefines PROPERTIES POSITION_INDEPENDENT_CODE ON)