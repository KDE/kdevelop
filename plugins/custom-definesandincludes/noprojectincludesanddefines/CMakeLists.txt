set( noprojectincludesanddefines_SRCS
        noprojectcustomincludepaths.cpp
        noprojectincludepathsmanager.cpp
   )

ki18n_wrap_ui( noprojectincludesanddefines_SRCS
        noprojectcustomincludepaths.ui )

add_library(kdevnoprojectincludesanddefines STATIC
        ${noprojectincludesanddefines_SRCS}
    )

target_link_libraries( kdevnoprojectincludesanddefines
    PRIVATE
        KDev::Util
        KDev::Language
        KF6::KIOWidgets
)
