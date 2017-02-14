include_directories(lib)

set(kdevastyle_PART_SRCS
    astyle_plugin.cpp
    astyle_preferences.cpp
    astyle_formatter.cpp
    astyle_stringiterator.cpp
    debug.cpp
)

set(astyle_preferences_UI
    astyle_preferences.ui
)

ki18n_wrap_ui(kdevastyle_PART_SRCS ${astyle_preferences_UI} )
kdevplatform_add_plugin(kdevastyle JSON kdevastyle.json SOURCES ${kdevastyle_PART_SRCS})
target_link_libraries(kdevastyle
    astylelib

    KF5::Parts
    KF5::KIOWidgets
    KF5::TextEditor

    KDev::Interfaces
    KDev::Util)

add_subdirectory(lib)
add_subdirectory(tests)
