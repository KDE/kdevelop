set(astyletest_SRCS astyletest.cpp
  ../astyle_formatter.cpp
  ../lib/ASFormatter.cpp
  ../lib/ASResource.cpp
  ../lib/ASEnhancer.cpp
  ../lib/ASBeautifier.cpp
  ../lib/astyle_stringiterator.cpp
)

kde4_add_unit_test(astyletest ${astyletest_SRCS})
target_link_libraries(astyletest
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_INTERFACES_LIBRARIES}
  ${KDEVPLATFORM_UTIL_LIBRARIES}
)

