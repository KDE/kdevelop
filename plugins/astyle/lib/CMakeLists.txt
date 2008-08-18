set(astylewrapper_LIB_SRCS
    ASBeautifier.cpp
    ASEnhancer.cpp
    ASFormatter.cpp
    ASResource.cpp
    astyle_stringiterator.cpp
    astyle_formatter.cpp
)

kde4_add_library(astylewrapper SHARED ${astylewrapper_LIB_SRCS})
target_link_libraries(astylewrapper
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS})

install(TARGETS astylewrapper DESTINATION ${LIB_INSTALL_DIR})