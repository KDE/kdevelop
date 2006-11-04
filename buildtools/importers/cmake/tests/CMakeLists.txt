if (KDE4_BUILD_TESTS)

include_directories(
    ${CMAKE_SOURCE_DIR}/buildtools/importers/cmake
    ${CMAKE_SOURCE_DIR}/buildtools/importers/cmake/parser
    )

set( folderxmltest_SRCS folderxmltest.cpp )
kde4_automoc(${folderxmltest_SRCS})
kde4_add_executable(cmake-folderxmltest NOGUI RUN_UNINSTALLED ${folderxmltest_SRCS})
target_link_libraries(cmake-folderxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( targetxmltest_SRCS targetxmltest.cpp )
kde4_automoc(${targetxmltest_SRCS})
kde4_add_executable(cmake-targetxmltest NOGUI RUN_UNINSTALLED ${targetxmltest_SRCS})
target_link_libraries(cmake-targetxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( projectxmltest_SRCS projectxmltest.cpp )
kde4_automoc(${projectxmltest_SRCS})
kde4_add_executable( cmake-projectxmltest NOGUI RUN_UNINSTALLED ${projectxmltest_SRCS} )
target_link_libraries( cmake-projectxmltest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( cmakeparsertest_SRCS cmakeparsertest.cpp )
kde4_automoc( cmake-parsertest ${cmakeparsertest_SRCS} )
kde4_add_executable( cmake-parsertest NOGUI RUN_UNINSTALLED ${cmakeparsertest_SRCS} )
target_link_libraries( cmake-parsertest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( astfactorytest_SRCS astfactorytest.cpp )
kde4_automoc( cmake-astfactorytest ${astfactorytest_SRCS} )
kde4_add_executable( cmake-astfactorytest NOGUI RUN_UNINSTALLED ${astfactorytest_SRCS} )
target_link_libraries( cmake-astfactorytest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( customcommandasttest_SRCS cmakecustomcommandasttest.cpp )
kde4_automoc( cmake-customcommandasttest ${customcommandasttest_SRCS} )
kde4_add_executable( cmake-customcommandasttest NOGUI RUN_UNINSTALLED 
                     ${customcommandasttest_SRCS} )
target_link_libraries( cmake-customcommandasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( customtargetasttest_SRCS cmakecustomtargetasttest.cpp )
kde4_automoc( cmake-customtargetasttest ${customtargetasttest_SRCS} )
kde4_add_executable( cmake-customtargetasttest NOGUI RUN_UNINSTALLED
                     ${customtargetasttest_SRCS} )
target_link_libraries( cmake-customtargetasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( adddefinitiontest_SRCS cmakeadddefinitionasttest.cpp )
kde4_automoc( cmake-adddefinitionasttest ${adddefinitiontest_SRCS} )
kde4_add_executable( cmake-adddefinitionasttest NOGUI RUN_UNINSTALLED
                     ${adddefinitiontest_SRCS} )
target_link_libraries( cmake-adddefinitionasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

set( adddependenciestest_SRCS cmakeadddependenciestest.cpp )
kde4_automoc( cmake-adddependenciesasttest ${adddependenciestest_SRCS} )
kde4_add_executable( cmake-adddependenciesasttest NOGUI RUN_UNINSTALLED
                     ${adddependenciestest_SRCS} )
target_link_libraries( cmake-adddependenciesasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevcmakecommon
)

add_test(cmake-folderxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-folderxmltest)
add_test(cmake-targetxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-targetxmltest)
add_test(cmake-projectxmltest ${EXECUTABLE_OUTPUT_PATH}/cmake-projectxmltest)
add_test(cmake-parsertest ${EXECUTABLE_OUTPUT_PATH}/cmake-parsertest)
add_test(cmake-astfactorytest ${EXECUTABLE_OUTPUT_PATH}/cmake-astfactorytest)
add_test(cmake-customcommandtest ${EXECUTABLE_OUTPUT_PATH}/cmake-customcommandasttest)
add_test(cmake-customtargetasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-customtargetasttest)
add_test(cmake-adddefinitionasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-adddefinitionasttest)
add_test(cmake-adddependenciesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-adddependenciesasttest)
set ( addexecutableasttest_SRCS cmake_addexecutableast_test.cpp )
kde4_automoc( cmake-addexecutableasttest ${addexecutableasttest_SRCS} )
kde4_add_executable( cmake-addexecutableasttest NOGUI RUN_UNINSTALLED ${addexecutableasttest_SRCS} )
target_link_libraries( cmake-addexecutableasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-addexecutableasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-addexecutableasttest )
set ( addlibraryasttest_SRCS cmake_addlibraryast_test.cpp )
kde4_automoc( cmake-addlibraryasttest ${addlibraryasttest_SRCS} )
kde4_add_executable( cmake-addlibraryasttest NOGUI RUN_UNINSTALLED ${addlibraryasttest_SRCS} )
target_link_libraries( cmake-addlibraryasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-addlibraryasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-addlibraryasttest )
set ( addsubdirectoryasttest_SRCS cmake_addsubdirectoryast_test.cpp )
kde4_automoc( cmake-addsubdirectoryasttest ${addsubdirectoryasttest_SRCS} )
kde4_add_executable( cmake-addsubdirectoryasttest NOGUI RUN_UNINSTALLED ${addsubdirectoryasttest_SRCS} )
target_link_libraries( cmake-addsubdirectoryasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-addsubdirectoryasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-addsubdirectoryasttest )
set ( addtestasttest_SRCS cmake_addtestast_test.cpp )
kde4_automoc( cmake-addtestasttest ${addtestasttest_SRCS} )
kde4_add_executable( cmake-addtestasttest NOGUI RUN_UNINSTALLED ${addtestasttest_SRCS} )
target_link_libraries( cmake-addtestasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-addtestasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-addtestasttest )
set ( auxsourcedirectoryasttest_SRCS cmake_auxsourcedirectoryast_test.cpp )
kde4_automoc( cmake-auxsourcedirectoryasttest ${auxsourcedirectoryasttest_SRCS} )
kde4_add_executable( cmake-auxsourcedirectoryasttest NOGUI RUN_UNINSTALLED ${auxsourcedirectoryasttest_SRCS} )
target_link_libraries( cmake-auxsourcedirectoryasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-auxsourcedirectoryasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-auxsourcedirectoryasttest )
set ( buildcommandasttest_SRCS cmake_buildcommandast_test.cpp )
kde4_automoc( cmake-buildcommandasttest ${buildcommandasttest_SRCS} )
kde4_add_executable( cmake-buildcommandasttest NOGUI RUN_UNINSTALLED ${buildcommandasttest_SRCS} )
target_link_libraries( cmake-buildcommandasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-buildcommandasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-buildcommandasttest )
set ( buildnameasttest_SRCS cmake_buildnameast_test.cpp )
kde4_automoc( cmake-buildnameasttest ${buildnameasttest_SRCS} )
kde4_add_executable( cmake-buildnameasttest NOGUI RUN_UNINSTALLED ${buildnameasttest_SRCS} )
target_link_libraries( cmake-buildnameasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-buildnameasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-buildnameasttest )
set ( cmakeminimumrequiredasttest_SRCS cmake_cmakeminimumrequiredast_test.cpp )
kde4_automoc( cmake-cmakeminimumrequiredasttest ${cmakeminimumrequiredasttest_SRCS} )
kde4_add_executable( cmake-cmakeminimumrequiredasttest NOGUI RUN_UNINSTALLED ${cmakeminimumrequiredasttest_SRCS} )
target_link_libraries( cmake-cmakeminimumrequiredasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-cmakeminimumrequiredasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-cmakeminimumrequiredasttest )
set ( configurefileasttest_SRCS cmake_configurefileast_test.cpp )
kde4_automoc( cmake-configurefileasttest ${configurefileasttest_SRCS} )
kde4_add_executable( cmake-configurefileasttest NOGUI RUN_UNINSTALLED ${configurefileasttest_SRCS} )
target_link_libraries( cmake-configurefileasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-configurefileasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-configurefileasttest )
set ( createtestsourcelistasttest_SRCS cmake_createtestsourcelistast_test.cpp )
kde4_automoc( cmake-createtestsourcelistasttest ${createtestsourcelistasttest_SRCS} )
kde4_add_executable( cmake-createtestsourcelistasttest NOGUI RUN_UNINSTALLED ${createtestsourcelistasttest_SRCS} )
target_link_libraries( cmake-createtestsourcelistasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-createtestsourcelistasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-createtestsourcelistasttest )
set ( enablelanguageasttest_SRCS cmake_enablelanguageast_test.cpp )
kde4_automoc( cmake-enablelanguageasttest ${enablelanguageasttest_SRCS} )
kde4_add_executable( cmake-enablelanguageasttest NOGUI RUN_UNINSTALLED ${enablelanguageasttest_SRCS} )
target_link_libraries( cmake-enablelanguageasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-enablelanguageasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-enablelanguageasttest )
set ( enabletestingasttest_SRCS cmake_enabletestingast_test.cpp )
kde4_automoc( cmake-enabletestingasttest ${enabletestingasttest_SRCS} )
kde4_add_executable( cmake-enabletestingasttest NOGUI RUN_UNINSTALLED ${enabletestingasttest_SRCS} )
target_link_libraries( cmake-enabletestingasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-enabletestingasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-enabletestingasttest )
set ( execprogramasttest_SRCS cmake_execprogramast_test.cpp )
kde4_automoc( cmake-execprogramasttest ${execprogramasttest_SRCS} )
kde4_add_executable( cmake-execprogramasttest NOGUI RUN_UNINSTALLED ${execprogramasttest_SRCS} )
target_link_libraries( cmake-execprogramasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-execprogramasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-execprogramasttest )
set ( executeprocessasttest_SRCS cmake_executeprocessast_test.cpp )
kde4_automoc( cmake-executeprocessasttest ${executeprocessasttest_SRCS} )
kde4_add_executable( cmake-executeprocessasttest NOGUI RUN_UNINSTALLED ${executeprocessasttest_SRCS} )
target_link_libraries( cmake-executeprocessasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-executeprocessasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-executeprocessasttest )
set ( exportlibrarydepsasttest_SRCS cmake_exportlibrarydepsast_test.cpp )
kde4_automoc( cmake-exportlibrarydepsasttest ${exportlibrarydepsasttest_SRCS} )
kde4_add_executable( cmake-exportlibrarydepsasttest NOGUI RUN_UNINSTALLED ${exportlibrarydepsasttest_SRCS} )
target_link_libraries( cmake-exportlibrarydepsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-exportlibrarydepsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-exportlibrarydepsasttest )
set ( fileasttest_SRCS cmake_fileast_test.cpp )
kde4_automoc( cmake-fileasttest ${fileasttest_SRCS} )
kde4_add_executable( cmake-fileasttest NOGUI RUN_UNINSTALLED ${fileasttest_SRCS} )
target_link_libraries( cmake-fileasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-fileasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-fileasttest )
set ( findfileasttest_SRCS cmake_findfileast_test.cpp )
kde4_automoc( cmake-findfileasttest ${findfileasttest_SRCS} )
kde4_add_executable( cmake-findfileasttest NOGUI RUN_UNINSTALLED ${findfileasttest_SRCS} )
target_link_libraries( cmake-findfileasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-findfileasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-findfileasttest )
set ( findlibraryasttest_SRCS cmake_findlibraryast_test.cpp )
kde4_automoc( cmake-findlibraryasttest ${findlibraryasttest_SRCS} )
kde4_add_executable( cmake-findlibraryasttest NOGUI RUN_UNINSTALLED ${findlibraryasttest_SRCS} )
target_link_libraries( cmake-findlibraryasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-findlibraryasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-findlibraryasttest )
set ( findpackageasttest_SRCS cmake_findpackageast_test.cpp )
kde4_automoc( cmake-findpackageasttest ${findpackageasttest_SRCS} )
kde4_add_executable( cmake-findpackageasttest NOGUI RUN_UNINSTALLED ${findpackageasttest_SRCS} )
target_link_libraries( cmake-findpackageasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-findpackageasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-findpackageasttest )
set ( findpathasttest_SRCS cmake_findpathast_test.cpp )
kde4_automoc( cmake-findpathasttest ${findpathasttest_SRCS} )
kde4_add_executable( cmake-findpathasttest NOGUI RUN_UNINSTALLED ${findpathasttest_SRCS} )
target_link_libraries( cmake-findpathasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-findpathasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-findpathasttest )
set ( findprogramasttest_SRCS cmake_findprogramast_test.cpp )
kde4_automoc( cmake-findprogramasttest ${findprogramasttest_SRCS} )
kde4_add_executable( cmake-findprogramasttest NOGUI RUN_UNINSTALLED ${findprogramasttest_SRCS} )
target_link_libraries( cmake-findprogramasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-findprogramasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-findprogramasttest )
set ( fltkwrapuiasttest_SRCS cmake_fltkwrapuiast_test.cpp )
kde4_automoc( cmake-fltkwrapuiasttest ${fltkwrapuiasttest_SRCS} )
kde4_add_executable( cmake-fltkwrapuiasttest NOGUI RUN_UNINSTALLED ${fltkwrapuiasttest_SRCS} )
target_link_libraries( cmake-fltkwrapuiasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-fltkwrapuiasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-fltkwrapuiasttest )
set ( foreachasttest_SRCS cmake_foreachast_test.cpp )
kde4_automoc( cmake-foreachasttest ${foreachasttest_SRCS} )
kde4_add_executable( cmake-foreachasttest NOGUI RUN_UNINSTALLED ${foreachasttest_SRCS} )
target_link_libraries( cmake-foreachasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-foreachasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-foreachasttest )
set ( getcmakepropertyasttest_SRCS cmake_getcmakepropertyast_test.cpp )
kde4_automoc( cmake-getcmakepropertyasttest ${getcmakepropertyasttest_SRCS} )
kde4_add_executable( cmake-getcmakepropertyasttest NOGUI RUN_UNINSTALLED ${getcmakepropertyasttest_SRCS} )
target_link_libraries( cmake-getcmakepropertyasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-getcmakepropertyasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-getcmakepropertyasttest )
set ( getdirpropertyasttest_SRCS cmake_getdirpropertyast_test.cpp )
kde4_automoc( cmake-getdirpropertyasttest ${getdirpropertyasttest_SRCS} )
kde4_add_executable( cmake-getdirpropertyasttest NOGUI RUN_UNINSTALLED ${getdirpropertyasttest_SRCS} )
target_link_libraries( cmake-getdirpropertyasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-getdirpropertyasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-getdirpropertyasttest )
set ( getfilenamecomponentasttest_SRCS cmake_getfilenamecomponentast_test.cpp )
kde4_automoc( cmake-getfilenamecomponentasttest ${getfilenamecomponentasttest_SRCS} )
kde4_add_executable( cmake-getfilenamecomponentasttest NOGUI RUN_UNINSTALLED ${getfilenamecomponentasttest_SRCS} )
target_link_libraries( cmake-getfilenamecomponentasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-getfilenamecomponentasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-getfilenamecomponentasttest )
set ( getsourcefilepropasttest_SRCS cmake_getsourcefilepropast_test.cpp )
kde4_automoc( cmake-getsourcefilepropasttest ${getsourcefilepropasttest_SRCS} )
kde4_add_executable( cmake-getsourcefilepropasttest NOGUI RUN_UNINSTALLED ${getsourcefilepropasttest_SRCS} )
target_link_libraries( cmake-getsourcefilepropasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-getsourcefilepropasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-getsourcefilepropasttest )
set ( gettargetpropasttest_SRCS cmake_gettargetpropast_test.cpp )
kde4_automoc( cmake-gettargetpropasttest ${gettargetpropasttest_SRCS} )
kde4_add_executable( cmake-gettargetpropasttest NOGUI RUN_UNINSTALLED ${gettargetpropasttest_SRCS} )
target_link_libraries( cmake-gettargetpropasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-gettargetpropasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-gettargetpropasttest )
set ( gettestpropasttest_SRCS cmake_gettestpropast_test.cpp )
kde4_automoc( cmake-gettestpropasttest ${gettestpropasttest_SRCS} )
kde4_add_executable( cmake-gettestpropasttest NOGUI RUN_UNINSTALLED ${gettestpropasttest_SRCS} )
target_link_libraries( cmake-gettestpropasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-gettestpropasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-gettestpropasttest )
set ( ifasttest_SRCS cmake_ifast_test.cpp )
kde4_automoc( cmake-ifasttest ${ifasttest_SRCS} )
kde4_add_executable( cmake-ifasttest NOGUI RUN_UNINSTALLED ${ifasttest_SRCS} )
target_link_libraries( cmake-ifasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-ifasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-ifasttest )
set ( includeasttest_SRCS cmake_includeast_test.cpp )
kde4_automoc( cmake-includeasttest ${includeasttest_SRCS} )
kde4_add_executable( cmake-includeasttest NOGUI RUN_UNINSTALLED ${includeasttest_SRCS} )
target_link_libraries( cmake-includeasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-includeasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-includeasttest )
set ( includedirectoriesasttest_SRCS cmake_includedirectoriesast_test.cpp )
kde4_automoc( cmake-includedirectoriesasttest ${includedirectoriesasttest_SRCS} )
kde4_add_executable( cmake-includedirectoriesasttest NOGUI RUN_UNINSTALLED ${includedirectoriesasttest_SRCS} )
target_link_libraries( cmake-includedirectoriesasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-includedirectoriesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-includedirectoriesasttest )
set ( includeexternalmsprojectasttest_SRCS cmake_includeexternalmsprojectast_test.cpp )
kde4_automoc( cmake-includeexternalmsprojectasttest ${includeexternalmsprojectasttest_SRCS} )
kde4_add_executable( cmake-includeexternalmsprojectasttest NOGUI RUN_UNINSTALLED ${includeexternalmsprojectasttest_SRCS} )
target_link_libraries( cmake-includeexternalmsprojectasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-includeexternalmsprojectasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-includeexternalmsprojectasttest )
set ( includeregularexpressionasttest_SRCS cmake_includeregularexpressionast_test.cpp )
kde4_automoc( cmake-includeregularexpressionasttest ${includeregularexpressionasttest_SRCS} )
kde4_add_executable( cmake-includeregularexpressionasttest NOGUI RUN_UNINSTALLED ${includeregularexpressionasttest_SRCS} )
target_link_libraries( cmake-includeregularexpressionasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-includeregularexpressionasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-includeregularexpressionasttest )
set ( installasttest_SRCS cmake_installast_test.cpp )
kde4_automoc( cmake-installasttest ${installasttest_SRCS} )
kde4_add_executable( cmake-installasttest NOGUI RUN_UNINSTALLED ${installasttest_SRCS} )
target_link_libraries( cmake-installasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-installasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-installasttest )
set ( installfilesasttest_SRCS cmake_installfilesast_test.cpp )
kde4_automoc( cmake-installfilesasttest ${installfilesasttest_SRCS} )
kde4_add_executable( cmake-installfilesasttest NOGUI RUN_UNINSTALLED ${installfilesasttest_SRCS} )
target_link_libraries( cmake-installfilesasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-installfilesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-installfilesasttest )
set ( installprogramsasttest_SRCS cmake_installprogramsast_test.cpp )
kde4_automoc( cmake-installprogramsasttest ${installprogramsasttest_SRCS} )
kde4_add_executable( cmake-installprogramsasttest NOGUI RUN_UNINSTALLED ${installprogramsasttest_SRCS} )
target_link_libraries( cmake-installprogramsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-installprogramsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-installprogramsasttest )
set ( installtargetsasttest_SRCS cmake_installtargetsast_test.cpp )
kde4_automoc( cmake-installtargetsasttest ${installtargetsasttest_SRCS} )
kde4_add_executable( cmake-installtargetsasttest NOGUI RUN_UNINSTALLED ${installtargetsasttest_SRCS} )
target_link_libraries( cmake-installtargetsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-installtargetsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-installtargetsasttest )
set ( linkdirectoriesasttest_SRCS cmake_linkdirectoriesast_test.cpp )
kde4_automoc( cmake-linkdirectoriesasttest ${linkdirectoriesasttest_SRCS} )
kde4_add_executable( cmake-linkdirectoriesasttest NOGUI RUN_UNINSTALLED ${linkdirectoriesasttest_SRCS} )
target_link_libraries( cmake-linkdirectoriesasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-linkdirectoriesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-linkdirectoriesasttest )
set ( linklibrariesasttest_SRCS cmake_linklibrariesast_test.cpp )
kde4_automoc( cmake-linklibrariesasttest ${linklibrariesasttest_SRCS} )
kde4_add_executable( cmake-linklibrariesasttest NOGUI RUN_UNINSTALLED ${linklibrariesasttest_SRCS} )
target_link_libraries( cmake-linklibrariesasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-linklibrariesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-linklibrariesasttest )
set ( listasttest_SRCS cmake_listast_test.cpp )
kde4_automoc( cmake-listasttest ${listasttest_SRCS} )
kde4_add_executable( cmake-listasttest NOGUI RUN_UNINSTALLED ${listasttest_SRCS} )
target_link_libraries( cmake-listasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-listasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-listasttest )
set ( loadcacheasttest_SRCS cmake_loadcacheast_test.cpp )
kde4_automoc( cmake-loadcacheasttest ${loadcacheasttest_SRCS} )
kde4_add_executable( cmake-loadcacheasttest NOGUI RUN_UNINSTALLED ${loadcacheasttest_SRCS} )
target_link_libraries( cmake-loadcacheasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-loadcacheasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-loadcacheasttest )
set ( loadcommandasttest_SRCS cmake_loadcommandast_test.cpp )
kde4_automoc( cmake-loadcommandasttest ${loadcommandasttest_SRCS} )
kde4_add_executable( cmake-loadcommandasttest NOGUI RUN_UNINSTALLED ${loadcommandasttest_SRCS} )
target_link_libraries( cmake-loadcommandasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-loadcommandasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-loadcommandasttest )
set ( macroasttest_SRCS cmake_macroast_test.cpp )
kde4_automoc( cmake-macroasttest ${macroasttest_SRCS} )
kde4_add_executable( cmake-macroasttest NOGUI RUN_UNINSTALLED ${macroasttest_SRCS} )
target_link_libraries( cmake-macroasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-macroasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-macroasttest )
set ( makedirectoryasttest_SRCS cmake_makedirectoryast_test.cpp )
kde4_automoc( cmake-makedirectoryasttest ${makedirectoryasttest_SRCS} )
kde4_add_executable( cmake-makedirectoryasttest NOGUI RUN_UNINSTALLED ${makedirectoryasttest_SRCS} )
target_link_libraries( cmake-makedirectoryasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-makedirectoryasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-makedirectoryasttest )
set ( markasadvancedasttest_SRCS cmake_markasadvancedast_test.cpp )
kde4_automoc( cmake-markasadvancedasttest ${markasadvancedasttest_SRCS} )
kde4_add_executable( cmake-markasadvancedasttest NOGUI RUN_UNINSTALLED ${markasadvancedasttest_SRCS} )
target_link_libraries( cmake-markasadvancedasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-markasadvancedasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-markasadvancedasttest )
set ( mathasttest_SRCS cmake_mathast_test.cpp )
kde4_automoc( cmake-mathasttest ${mathasttest_SRCS} )
kde4_add_executable( cmake-mathasttest NOGUI RUN_UNINSTALLED ${mathasttest_SRCS} )
target_link_libraries( cmake-mathasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-mathasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-mathasttest )
set ( messageasttest_SRCS cmake_messageast_test.cpp )
kde4_automoc( cmake-messageasttest ${messageasttest_SRCS} )
kde4_add_executable( cmake-messageasttest NOGUI RUN_UNINSTALLED ${messageasttest_SRCS} )
target_link_libraries( cmake-messageasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-messageasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-messageasttest )
set ( optionasttest_SRCS cmake_optionast_test.cpp )
kde4_automoc( cmake-optionasttest ${optionasttest_SRCS} )
kde4_add_executable( cmake-optionasttest NOGUI RUN_UNINSTALLED ${optionasttest_SRCS} )
target_link_libraries( cmake-optionasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-optionasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-optionasttest )
set ( outputrequiredfilesasttest_SRCS cmake_outputrequiredfilesast_test.cpp )
kde4_automoc( cmake-outputrequiredfilesasttest ${outputrequiredfilesasttest_SRCS} )
kde4_add_executable( cmake-outputrequiredfilesasttest NOGUI RUN_UNINSTALLED ${outputrequiredfilesasttest_SRCS} )
target_link_libraries( cmake-outputrequiredfilesasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-outputrequiredfilesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-outputrequiredfilesasttest )
set ( projectasttest_SRCS cmake_projectast_test.cpp )
kde4_automoc( cmake-projectasttest ${projectasttest_SRCS} )
kde4_add_executable( cmake-projectasttest NOGUI RUN_UNINSTALLED ${projectasttest_SRCS} )
target_link_libraries( cmake-projectasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-projectasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-projectasttest )
set ( qtwrapcppasttest_SRCS cmake_qtwrapcppast_test.cpp )
kde4_automoc( cmake-qtwrapcppasttest ${qtwrapcppasttest_SRCS} )
kde4_add_executable( cmake-qtwrapcppasttest NOGUI RUN_UNINSTALLED ${qtwrapcppasttest_SRCS} )
target_link_libraries( cmake-qtwrapcppasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-qtwrapcppasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-qtwrapcppasttest )
set ( qtwrapuiasttest_SRCS cmake_qtwrapuiast_test.cpp )
kde4_automoc( cmake-qtwrapuiasttest ${qtwrapuiasttest_SRCS} )
kde4_add_executable( cmake-qtwrapuiasttest NOGUI RUN_UNINSTALLED ${qtwrapuiasttest_SRCS} )
target_link_libraries( cmake-qtwrapuiasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-qtwrapuiasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-qtwrapuiasttest )
set ( removeasttest_SRCS cmake_removeast_test.cpp )
kde4_automoc( cmake-removeasttest ${removeasttest_SRCS} )
kde4_add_executable( cmake-removeasttest NOGUI RUN_UNINSTALLED ${removeasttest_SRCS} )
target_link_libraries( cmake-removeasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-removeasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-removeasttest )
set ( removedefinitionsasttest_SRCS cmake_removedefinitionsast_test.cpp )
kde4_automoc( cmake-removedefinitionsasttest ${removedefinitionsasttest_SRCS} )
kde4_add_executable( cmake-removedefinitionsasttest NOGUI RUN_UNINSTALLED ${removedefinitionsasttest_SRCS} )
target_link_libraries( cmake-removedefinitionsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-removedefinitionsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-removedefinitionsasttest )
set ( separateargumentsasttest_SRCS cmake_separateargumentsast_test.cpp )
kde4_automoc( cmake-separateargumentsasttest ${separateargumentsasttest_SRCS} )
kde4_add_executable( cmake-separateargumentsasttest NOGUI RUN_UNINSTALLED ${separateargumentsasttest_SRCS} )
target_link_libraries( cmake-separateargumentsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-separateargumentsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-separateargumentsasttest )
set ( setasttest_SRCS cmake_setast_test.cpp )
kde4_automoc( cmake-setasttest ${setasttest_SRCS} )
kde4_add_executable( cmake-setasttest NOGUI RUN_UNINSTALLED ${setasttest_SRCS} )
target_link_libraries( cmake-setasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-setasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-setasttest )
set ( setdirectorypropsasttest_SRCS cmake_setdirectorypropsast_test.cpp )
kde4_automoc( cmake-setdirectorypropsasttest ${setdirectorypropsasttest_SRCS} )
kde4_add_executable( cmake-setdirectorypropsasttest NOGUI RUN_UNINSTALLED ${setdirectorypropsasttest_SRCS} )
target_link_libraries( cmake-setdirectorypropsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-setdirectorypropsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-setdirectorypropsasttest )
set ( setsourcefilespropsasttest_SRCS cmake_setsourcefilespropsast_test.cpp )
kde4_automoc( cmake-setsourcefilespropsasttest ${setsourcefilespropsasttest_SRCS} )
kde4_add_executable( cmake-setsourcefilespropsasttest NOGUI RUN_UNINSTALLED ${setsourcefilespropsasttest_SRCS} )
target_link_libraries( cmake-setsourcefilespropsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-setsourcefilespropsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-setsourcefilespropsasttest )
set ( settargetpropsasttest_SRCS cmake_settargetpropsast_test.cpp )
kde4_automoc( cmake-settargetpropsasttest ${settargetpropsasttest_SRCS} )
kde4_add_executable( cmake-settargetpropsasttest NOGUI RUN_UNINSTALLED ${settargetpropsasttest_SRCS} )
target_link_libraries( cmake-settargetpropsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-settargetpropsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-settargetpropsasttest )
set ( settestspropsasttest_SRCS cmake_settestspropsast_test.cpp )
kde4_automoc( cmake-settestspropsasttest ${settestspropsasttest_SRCS} )
kde4_add_executable( cmake-settestspropsasttest NOGUI RUN_UNINSTALLED ${settestspropsasttest_SRCS} )
target_link_libraries( cmake-settestspropsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-settestspropsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-settestspropsasttest )
set ( sitenameasttest_SRCS cmake_sitenameast_test.cpp )
kde4_automoc( cmake-sitenameasttest ${sitenameasttest_SRCS} )
kde4_add_executable( cmake-sitenameasttest NOGUI RUN_UNINSTALLED ${sitenameasttest_SRCS} )
target_link_libraries( cmake-sitenameasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-sitenameasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-sitenameasttest )
set ( sourcegroupasttest_SRCS cmake_sourcegroupast_test.cpp )
kde4_automoc( cmake-sourcegroupasttest ${sourcegroupasttest_SRCS} )
kde4_add_executable( cmake-sourcegroupasttest NOGUI RUN_UNINSTALLED ${sourcegroupasttest_SRCS} )
target_link_libraries( cmake-sourcegroupasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-sourcegroupasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-sourcegroupasttest )
set ( stringasttest_SRCS cmake_stringast_test.cpp )
kde4_automoc( cmake-stringasttest ${stringasttest_SRCS} )
kde4_add_executable( cmake-stringasttest NOGUI RUN_UNINSTALLED ${stringasttest_SRCS} )
target_link_libraries( cmake-stringasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-stringasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-stringasttest )
set ( subdirdependsasttest_SRCS cmake_subdirdependsast_test.cpp )
kde4_automoc( cmake-subdirdependsasttest ${subdirdependsasttest_SRCS} )
kde4_add_executable( cmake-subdirdependsasttest NOGUI RUN_UNINSTALLED ${subdirdependsasttest_SRCS} )
target_link_libraries( cmake-subdirdependsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-subdirdependsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-subdirdependsasttest )
set ( subdirsasttest_SRCS cmake_subdirsast_test.cpp )
kde4_automoc( cmake-subdirsasttest ${subdirsasttest_SRCS} )
kde4_add_executable( cmake-subdirsasttest NOGUI RUN_UNINSTALLED ${subdirsasttest_SRCS} )
target_link_libraries( cmake-subdirsasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-subdirsasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-subdirsasttest )
set ( targetlinklibrariesasttest_SRCS cmake_targetlinklibrariesast_test.cpp )
kde4_automoc( cmake-targetlinklibrariesasttest ${targetlinklibrariesasttest_SRCS} )
kde4_add_executable( cmake-targetlinklibrariesasttest NOGUI RUN_UNINSTALLED ${targetlinklibrariesasttest_SRCS} )
target_link_libraries( cmake-targetlinklibrariesasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-targetlinklibrariesasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-targetlinklibrariesasttest )
set ( trycompileasttest_SRCS cmake_trycompileast_test.cpp )
kde4_automoc( cmake-trycompileasttest ${trycompileasttest_SRCS} )
kde4_add_executable( cmake-trycompileasttest NOGUI RUN_UNINSTALLED ${trycompileasttest_SRCS} )
target_link_libraries( cmake-trycompileasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-trycompileasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-trycompileasttest )
set ( tryrunasttest_SRCS cmake_tryrunast_test.cpp )
kde4_automoc( cmake-tryrunasttest ${tryrunasttest_SRCS} )
kde4_add_executable( cmake-tryrunasttest NOGUI RUN_UNINSTALLED ${tryrunasttest_SRCS} )
target_link_libraries( cmake-tryrunasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-tryrunasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-tryrunasttest )
set ( usemangledmesaasttest_SRCS cmake_usemangledmesaast_test.cpp )
kde4_automoc( cmake-usemangledmesaasttest ${usemangledmesaasttest_SRCS} )
kde4_add_executable( cmake-usemangledmesaasttest NOGUI RUN_UNINSTALLED ${usemangledmesaasttest_SRCS} )
target_link_libraries( cmake-usemangledmesaasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-usemangledmesaasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-usemangledmesaasttest )
set ( utilitysourceasttest_SRCS cmake_utilitysourceast_test.cpp )
kde4_automoc( cmake-utilitysourceasttest ${utilitysourceasttest_SRCS} )
kde4_add_executable( cmake-utilitysourceasttest NOGUI RUN_UNINSTALLED ${utilitysourceasttest_SRCS} )
target_link_libraries( cmake-utilitysourceasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-utilitysourceasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-utilitysourceasttest )
set ( variablerequiresasttest_SRCS cmake_variablerequiresast_test.cpp )
kde4_automoc( cmake-variablerequiresasttest ${variablerequiresasttest_SRCS} )
kde4_add_executable( cmake-variablerequiresasttest NOGUI RUN_UNINSTALLED ${variablerequiresasttest_SRCS} )
target_link_libraries( cmake-variablerequiresasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-variablerequiresasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-variablerequiresasttest )
set ( vtkmakeinstantiatorasttest_SRCS cmake_vtkmakeinstantiatorast_test.cpp )
kde4_automoc( cmake-vtkmakeinstantiatorasttest ${vtkmakeinstantiatorasttest_SRCS} )
kde4_add_executable( cmake-vtkmakeinstantiatorasttest NOGUI RUN_UNINSTALLED ${vtkmakeinstantiatorasttest_SRCS} )
target_link_libraries( cmake-vtkmakeinstantiatorasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-vtkmakeinstantiatorasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-vtkmakeinstantiatorasttest )
set ( vtkwrapjavaasttest_SRCS cmake_vtkwrapjavaast_test.cpp )
kde4_automoc( cmake-vtkwrapjavaasttest ${vtkwrapjavaasttest_SRCS} )
kde4_add_executable( cmake-vtkwrapjavaasttest NOGUI RUN_UNINSTALLED ${vtkwrapjavaasttest_SRCS} )
target_link_libraries( cmake-vtkwrapjavaasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-vtkwrapjavaasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-vtkwrapjavaasttest )
set ( vtkwrappythonasttest_SRCS cmake_vtkwrappythonast_test.cpp )
kde4_automoc( cmake-vtkwrappythonasttest ${vtkwrappythonasttest_SRCS} )
kde4_add_executable( cmake-vtkwrappythonasttest NOGUI RUN_UNINSTALLED ${vtkwrappythonasttest_SRCS} )
target_link_libraries( cmake-vtkwrappythonasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-vtkwrappythonasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-vtkwrappythonasttest )
set ( vtkwraptclasttest_SRCS cmake_vtkwraptclast_test.cpp )
kde4_automoc( cmake-vtkwraptclasttest ${vtkwraptclasttest_SRCS} )
kde4_add_executable( cmake-vtkwraptclasttest NOGUI RUN_UNINSTALLED ${vtkwraptclasttest_SRCS} )
target_link_libraries( cmake-vtkwraptclasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-vtkwraptclasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-vtkwraptclasttest )
set ( whileasttest_SRCS cmake_whileast_test.cpp )
kde4_automoc( cmake-whileasttest ${whileasttest_SRCS} )
kde4_add_executable( cmake-whileasttest NOGUI RUN_UNINSTALLED ${whileasttest_SRCS} )
target_link_libraries( cmake-whileasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-whileasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-whileasttest )
set ( writefileasttest_SRCS cmake_writefileast_test.cpp )
kde4_automoc( cmake-writefileasttest ${writefileasttest_SRCS} )
kde4_add_executable( cmake-writefileasttest NOGUI RUN_UNINSTALLED ${writefileasttest_SRCS} )
target_link_libraries( cmake-writefileasttest 
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevplatform
  kdevcmakecommon )
add_test( cmake-writefileasttest ${EXECUTABLE_OUTPUT_PATH}/cmake-writefileasttest )
endif(KDE4_BUILD_TESTS)




























































































































































