project(QxCppUnit)

find_package(KDE4 REQUIRED)

include_directories(${KDE4_INCLUDES} 
                    ${PROJECT_BINARY_DIR} 
                    ${PROJECT_SOURCE_DIR}/../ )

########### next target ###############

set(qxcppunit_SRCS
    cppunititem.cpp
    cppunitmodel.cpp
    testrunner.cpp)

qt4_add_resources( qxcppunit_SRCS ${PROJECT_SOURCE_DIR}/resources/qxcppunit.qrc )
qt4_automoc( ${qxcppunit_SRCS} )

kde4_add_library(qxcppunit STATIC ${qxcppunit_SRCS})
# using shared here triggers a flood of crap
target_link_libraries(qxcppunit ${CMAKE_BINARY_DIR}/lib/libqxrunner.a)
#target_link_libraries(qxcppunit /home/nix/KdeDev/kdevelop/plugins/xunitrunner/build/lib/libqxrunner.so.1.0.0 )
set_target_properties(qxcppunit PROPERTIES VERSION 1.0.0 SOVERSION 1 )

# this is just for external use, to be removed
#kde4_add_library(qxcppunit ${qxcppunit_SRCS})

install(TARGETS qxcppunit DESTINATION ${LIB_INSTALL_DIR} )

########### next target ###############

# TODO install qxcppunit headers to include when building the tests
