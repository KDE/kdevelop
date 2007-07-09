include_directories(
    ${CMAKE_SOURCE_DIR}/buildtools/managers/cmake
    ${CMAKE_SOURCE_DIR}/buildtools/managers/cmake/parser
    )

#set( folderxmltest_SRCS folderxmltest.cpp )

#kde4_add_unit_test(cmake-folderxmltest  ${folderxmltest_SRCS})
#target_link_libraries(cmake-folderxmltest
#    ${QT_QTTEST_LIBRARY}
#    ${KDE4_KDECORE_LIBS}
#    kdevcmakecommon
#)

#set( targetxmltest_SRCS targetxmltest.cpp )

#kde4_add_unit_test(cmake-targetxmltest  ${targetxmltest_SRCS})
#target_link_libraries(cmake-targetxmltest
#    ${QT_QTTEST_LIBRARY}
#    ${KDE4_KDECORE_LIBS}
#    kdevcmakecommon
#)

#set( projectxmltest_SRCS projectxmltest.cpp )

#kde4_add_unit_test(cmake-projectxmltest  ${projectxmltest_SRCS} )
#target_link_libraries( cmake-projectxmltest
#    ${QT_QTTEST_LIBRARY}
#    ${KDE4_KDECORE_LIBS}
#    kdevcmakecommon
#)

set( cmakeparsertest_SRCS cmakeparsertest.cpp )

kde4_add_unit_test(cmake-parsertest  ${cmakeparsertest_SRCS} )
target_link_libraries( cmake-parsertest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevcmakecommon
)

set( astfactorytest_SRCS astfactorytest.cpp )

kde4_add_unit_test(cmake-astfactorytest  ${astfactorytest_SRCS} )
target_link_libraries( cmake-astfactorytest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevcmakecommon
)

set( customcommandasttest_SRCS cmakecustomcommandasttest.cpp )

kde4_add_unit_test(cmake-customcommandasttest 
                     ${customcommandasttest_SRCS} )
target_link_libraries( cmake-customcommandasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevcmakecommon
)

set( customtargetasttest_SRCS cmakecustomtargetasttest.cpp )

kde4_add_unit_test(cmake-customtargetasttest 
                     ${customtargetasttest_SRCS} )
target_link_libraries( cmake-customtargetasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevcmakecommon
)

set( adddefinitiontest_SRCS cmakeadddefinitionasttest.cpp )

kde4_add_unit_test(cmake-adddefinitionasttest 
                     ${adddefinitiontest_SRCS} )
target_link_libraries( cmake-adddefinitionasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevcmakecommon
)

set( adddependenciestest_SRCS cmakeadddependenciestest.cpp )

kde4_add_unit_test(cmake-adddependenciesasttest 
                     ${adddependenciestest_SRCS} )
target_link_libraries( cmake-adddependenciesasttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevcmakecommon
)

set ( addexecutableasttest_SRCS cmake_addexecutableast_test.cpp )

kde4_add_unit_test(cmake-addexecutableasttest  ${addexecutableasttest_SRCS} )
target_link_libraries( cmake-addexecutableasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( addlibraryasttest_SRCS cmake_addlibraryast_test.cpp )

kde4_add_unit_test(cmake-addlibraryasttest  ${addlibraryasttest_SRCS} )
target_link_libraries( cmake-addlibraryasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( addsubdirectoryasttest_SRCS cmake_addsubdirectoryast_test.cpp )

kde4_add_unit_test(cmake-addsubdirectoryasttest  ${addsubdirectoryasttest_SRCS} )
target_link_libraries( cmake-addsubdirectoryasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( addtestasttest_SRCS cmake_addtestast_test.cpp )

kde4_add_unit_test(cmake-addtestasttest  ${addtestasttest_SRCS} )
target_link_libraries( cmake-addtestasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( auxsourcedirectoryasttest_SRCS cmake_auxsourcedirectoryast_test.cpp )

kde4_add_unit_test(cmake-auxsourcedirectoryasttest  ${auxsourcedirectoryasttest_SRCS} )
target_link_libraries( cmake-auxsourcedirectoryasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( buildcommandasttest_SRCS cmake_buildcommandast_test.cpp )

kde4_add_unit_test(cmake-buildcommandasttest  ${buildcommandasttest_SRCS} )
target_link_libraries( cmake-buildcommandasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( buildnameasttest_SRCS cmake_buildnameast_test.cpp )

kde4_add_unit_test(cmake-buildnameasttest  ${buildnameasttest_SRCS} )
target_link_libraries( cmake-buildnameasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( cmakeminimumrequiredasttest_SRCS cmake_cmakeminimumrequiredast_test.cpp )

kde4_add_unit_test(cmakeminimumrequiredasttest  ${cmakeminimumrequiredasttest_SRCS} )
target_link_libraries( cmakeminimumrequiredasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( configurefileasttest_SRCS cmake_configurefileast_test.cpp )

kde4_add_unit_test(cmake-configurefileasttest  ${configurefileasttest_SRCS} )
target_link_libraries( cmake-configurefileasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( createtestsourcelistasttest_SRCS cmake_createtestsourcelistast_test.cpp )

kde4_add_unit_test(cmake-createtestsourcelistasttest  ${createtestsourcelistasttest_SRCS} )
target_link_libraries( cmake-createtestsourcelistasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( enablelanguageasttest_SRCS cmake_enablelanguageast_test.cpp )

kde4_add_unit_test(cmake-enablelanguageasttest  ${enablelanguageasttest_SRCS} )
target_link_libraries( cmake-enablelanguageasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( enabletestingasttest_SRCS cmake_enabletestingast_test.cpp )

kde4_add_unit_test(cmake-enabletestingasttest  ${enabletestingasttest_SRCS} )
target_link_libraries( cmake-enabletestingasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( execprogramasttest_SRCS cmake_execprogramast_test.cpp )

kde4_add_unit_test(cmake-execprogramasttest  ${execprogramasttest_SRCS} )
target_link_libraries( cmake-execprogramasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( executeprocessasttest_SRCS cmake_executeprocessast_test.cpp )

kde4_add_unit_test(cmake-executeprocessasttest  ${executeprocessasttest_SRCS} )
target_link_libraries( cmake-executeprocessasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( exportlibrarydepsasttest_SRCS cmake_exportlibrarydepsast_test.cpp )

kde4_add_unit_test(cmake-exportlibrarydepsasttest  ${exportlibrarydepsasttest_SRCS} )
target_link_libraries( cmake-exportlibrarydepsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( fileasttest_SRCS cmake_fileast_test.cpp )

kde4_add_unit_test(cmake-fileasttest  ${fileasttest_SRCS} )
target_link_libraries( cmake-fileasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( findfileasttest_SRCS cmake_findfileast_test.cpp )

kde4_add_unit_test(cmake-findfileasttest  ${findfileasttest_SRCS} )
target_link_libraries( cmake-findfileasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( findlibraryasttest_SRCS cmake_findlibraryast_test.cpp )

kde4_add_unit_test(cmake-findlibraryasttest  ${findlibraryasttest_SRCS} )
target_link_libraries( cmake-findlibraryasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( findpackageasttest_SRCS cmake_findpackageast_test.cpp )

kde4_add_unit_test(cmake-findpackageasttest  ${findpackageasttest_SRCS} )
target_link_libraries( cmake-findpackageasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( findpathasttest_SRCS cmake_findpathast_test.cpp )

kde4_add_unit_test(cmake-findpathasttest  ${findpathasttest_SRCS} )
target_link_libraries( cmake-findpathasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( findprogramasttest_SRCS cmake_findprogramast_test.cpp )

kde4_add_unit_test(cmake-findprogramasttest  ${findprogramasttest_SRCS} )
target_link_libraries( cmake-findprogramasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( fltkwrapuiasttest_SRCS cmake_fltkwrapuiast_test.cpp )

kde4_add_unit_test(cmake-fltkwrapuiasttest  ${fltkwrapuiasttest_SRCS} )
target_link_libraries( cmake-fltkwrapuiasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( foreachasttest_SRCS cmake_foreachast_test.cpp )

kde4_add_unit_test(cmake-foreachasttest  ${foreachasttest_SRCS} )
target_link_libraries( cmake-foreachasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( getcmakepropertyasttest_SRCS cmake_getcmakepropertyast_test.cpp )

kde4_add_unit_test(cmake-getcmakepropertyasttest  ${getcmakepropertyasttest_SRCS} )
target_link_libraries( cmake-getcmakepropertyasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( getdirpropertyasttest_SRCS cmake_getdirpropertyast_test.cpp )

kde4_add_unit_test(cmake-getdirpropertyasttest  ${getdirpropertyasttest_SRCS} )
target_link_libraries( cmake-getdirpropertyasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( getfilenamecomponentasttest_SRCS cmake_getfilenamecomponentast_test.cpp )

kde4_add_unit_test(cmake-getfilenamecomponentasttest  ${getfilenamecomponentasttest_SRCS} )
target_link_libraries( cmake-getfilenamecomponentasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( getsourcefilepropasttest_SRCS cmake_getsourcefilepropast_test.cpp )

kde4_add_unit_test(cmake-getsourcefilepropasttest  ${getsourcefilepropasttest_SRCS} )
target_link_libraries( cmake-getsourcefilepropasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( gettargetpropasttest_SRCS cmake_gettargetpropast_test.cpp )

kde4_add_unit_test(cmake-gettargetpropasttest  ${gettargetpropasttest_SRCS} )
target_link_libraries( cmake-gettargetpropasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( gettestpropasttest_SRCS cmake_gettestpropast_test.cpp )

kde4_add_unit_test(cmake-gettestpropasttest  ${gettestpropasttest_SRCS} )
target_link_libraries( cmake-gettestpropasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( ifasttest_SRCS cmake_ifast_test.cpp )

kde4_add_unit_test(cmake-ifasttest  ${ifasttest_SRCS} )
target_link_libraries( cmake-ifasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( includeasttest_SRCS cmake_includeast_test.cpp )

kde4_add_unit_test(cmake-includeasttest  ${includeasttest_SRCS} )
target_link_libraries( cmake-includeasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( includedirectoriesasttest_SRCS cmake_includedirectoriesast_test.cpp )

kde4_add_unit_test(cmake-includedirectoriesasttest  ${includedirectoriesasttest_SRCS} )
target_link_libraries( cmake-includedirectoriesasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( includeexternalmsprojectasttest_SRCS cmake_includeexternalmsprojectast_test.cpp )

kde4_add_unit_test(cmake-includeexternalmsprojectasttest  ${includeexternalmsprojectasttest_SRCS} )
target_link_libraries( cmake-includeexternalmsprojectasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( includeregularexpressionasttest_SRCS cmake_includeregularexpressionast_test.cpp )

kde4_add_unit_test(cmake-includeregularexpressionasttest  ${includeregularexpressionasttest_SRCS} )
target_link_libraries( cmake-includeregularexpressionasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( installasttest_SRCS cmake_installast_test.cpp )

kde4_add_unit_test(cmake-installasttest  ${installasttest_SRCS} )
target_link_libraries( cmake-installasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( installfilesasttest_SRCS cmake_installfilesast_test.cpp )

kde4_add_unit_test(cmake-installfilesasttest  ${installfilesasttest_SRCS} )
target_link_libraries( cmake-installfilesasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( installprogramsasttest_SRCS cmake_installprogramsast_test.cpp )

kde4_add_unit_test(cmake-installprogramsasttest  ${installprogramsasttest_SRCS} )
target_link_libraries( cmake-installprogramsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( installtargetsasttest_SRCS cmake_installtargetsast_test.cpp )

kde4_add_unit_test(cmake-installtargetsasttest  ${installtargetsasttest_SRCS} )
target_link_libraries( cmake-installtargetsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( linkdirectoriesasttest_SRCS cmake_linkdirectoriesast_test.cpp )

kde4_add_unit_test(cmake-linkdirectoriesasttest  ${linkdirectoriesasttest_SRCS} )
target_link_libraries( cmake-linkdirectoriesasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( linklibrariesasttest_SRCS cmake_linklibrariesast_test.cpp )

kde4_add_unit_test(cmake-linklibrariesasttest  ${linklibrariesasttest_SRCS} )
target_link_libraries( cmake-linklibrariesasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( listasttest_SRCS cmake_listast_test.cpp )

kde4_add_unit_test(cmake-listasttest  ${listasttest_SRCS} )
target_link_libraries( cmake-listasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( loadcacheasttest_SRCS cmake_loadcacheast_test.cpp )

kde4_add_unit_test(cmake-loadcacheasttest  ${loadcacheasttest_SRCS} )
target_link_libraries( cmake-loadcacheasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( loadcommandasttest_SRCS cmake_loadcommandast_test.cpp )

kde4_add_unit_test(cmake-loadcommandasttest  ${loadcommandasttest_SRCS} )
target_link_libraries( cmake-loadcommandasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( macroasttest_SRCS cmake_macroast_test.cpp )

kde4_add_unit_test(cmake-macroasttest  ${macroasttest_SRCS} )
target_link_libraries( cmake-macroasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( makedirectoryasttest_SRCS cmake_makedirectoryast_test.cpp )

kde4_add_unit_test(cmake-makedirectoryasttest  ${makedirectoryasttest_SRCS} )
target_link_libraries( cmake-makedirectoryasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( markasadvancedasttest_SRCS cmake_markasadvancedast_test.cpp )

kde4_add_unit_test(cmake-markasadvancedasttest  ${markasadvancedasttest_SRCS} )
target_link_libraries( cmake-markasadvancedasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( mathasttest_SRCS cmake_mathast_test.cpp )

kde4_add_unit_test(cmake-mathasttest  ${mathasttest_SRCS} )
target_link_libraries( cmake-mathasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( messageasttest_SRCS cmake_messageast_test.cpp )

kde4_add_unit_test(cmake-messageasttest  ${messageasttest_SRCS} )
target_link_libraries( cmake-messageasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( optionasttest_SRCS cmake_optionast_test.cpp )

kde4_add_unit_test(cmake-optionasttest  ${optionasttest_SRCS} )
target_link_libraries( cmake-optionasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( outputrequiredfilesasttest_SRCS cmake_outputrequiredfilesast_test.cpp )

kde4_add_unit_test(cmake-outputrequiredfilesasttest  ${outputrequiredfilesasttest_SRCS} )
target_link_libraries( cmake-outputrequiredfilesasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( projectasttest_SRCS cmake_projectast_test.cpp )

kde4_add_unit_test(cmake-projectasttest  ${projectasttest_SRCS} )
target_link_libraries( cmake-projectasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( qtwrapcppasttest_SRCS cmake_qtwrapcppast_test.cpp )

kde4_add_unit_test(cmake-qtwrapcppasttest  ${qtwrapcppasttest_SRCS} )
target_link_libraries( cmake-qtwrapcppasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( qtwrapuiasttest_SRCS cmake_qtwrapuiast_test.cpp )

kde4_add_unit_test(cmake-qtwrapuiasttest  ${qtwrapuiasttest_SRCS} )
target_link_libraries( cmake-qtwrapuiasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( removeasttest_SRCS cmake_removeast_test.cpp )

kde4_add_unit_test(cmake-removeasttest  ${removeasttest_SRCS} )
target_link_libraries( cmake-removeasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( removedefinitionsasttest_SRCS cmake_removedefinitionsast_test.cpp )

kde4_add_unit_test(cmake-removedefinitionsasttest  ${removedefinitionsasttest_SRCS} )
target_link_libraries( cmake-removedefinitionsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( separateargumentsasttest_SRCS cmake_separateargumentsast_test.cpp )

kde4_add_unit_test(cmake-separateargumentsasttest  ${separateargumentsasttest_SRCS} )
target_link_libraries( cmake-separateargumentsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( setasttest_SRCS cmake_setast_test.cpp )

kde4_add_unit_test(cmake-setasttest  ${setasttest_SRCS} )
target_link_libraries( cmake-setasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( setdirectorypropsasttest_SRCS cmake_setdirectorypropsast_test.cpp )

kde4_add_unit_test(cmake-setdirectorypropsasttest  ${setdirectorypropsasttest_SRCS} )
target_link_libraries( cmake-setdirectorypropsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( setsourcefilespropsasttest_SRCS cmake_setsourcefilespropsast_test.cpp )

kde4_add_unit_test(cmake-setsourcefilespropsasttest  ${setsourcefilespropsasttest_SRCS} )
target_link_libraries( cmake-setsourcefilespropsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( settargetpropsasttest_SRCS cmake_settargetpropsast_test.cpp )

kde4_add_unit_test(cmake-settargetpropsasttest  ${settargetpropsasttest_SRCS} )
target_link_libraries( cmake-settargetpropsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( settestspropsasttest_SRCS cmake_settestspropsast_test.cpp )

kde4_add_unit_test(cmake-settestspropsasttest  ${settestspropsasttest_SRCS} )
target_link_libraries( cmake-settestspropsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( sitenameasttest_SRCS cmake_sitenameast_test.cpp )

kde4_add_unit_test(cmake-sitenameasttest  ${sitenameasttest_SRCS} )
target_link_libraries( cmake-sitenameasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( sourcegroupasttest_SRCS cmake_sourcegroupast_test.cpp )

kde4_add_unit_test(cmake-sourcegroupasttest  ${sourcegroupasttest_SRCS} )
target_link_libraries( cmake-sourcegroupasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( stringasttest_SRCS cmake_stringast_test.cpp )

kde4_add_unit_test(cmake-stringasttest  ${stringasttest_SRCS} )
target_link_libraries( cmake-stringasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( subdirdependsasttest_SRCS cmake_subdirdependsast_test.cpp )

kde4_add_unit_test(cmake-subdirdependsasttest  ${subdirdependsasttest_SRCS} )
target_link_libraries( cmake-subdirdependsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( subdirsasttest_SRCS cmake_subdirsast_test.cpp )

kde4_add_unit_test(cmake-subdirsasttest  ${subdirsasttest_SRCS} )
target_link_libraries( cmake-subdirsasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( targetlinklibrariesasttest_SRCS cmake_targetlinklibrariesast_test.cpp )

kde4_add_unit_test(cmake-targetlinklibrariesasttest  ${targetlinklibrariesasttest_SRCS} )
target_link_libraries( cmake-targetlinklibrariesasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( trycompileasttest_SRCS cmake_trycompileast_test.cpp )

kde4_add_unit_test(cmake-trycompileasttest  ${trycompileasttest_SRCS} )
target_link_libraries( cmake-trycompileasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( tryrunasttest_SRCS cmake_tryrunast_test.cpp )

kde4_add_unit_test(cmake-tryrunasttest  ${tryrunasttest_SRCS} )
target_link_libraries( cmake-tryrunasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( usemangledmesaasttest_SRCS cmake_usemangledmesaast_test.cpp )

kde4_add_unit_test(cmake-usemangledmesaasttest  ${usemangledmesaasttest_SRCS} )
target_link_libraries( cmake-usemangledmesaasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( utilitysourceasttest_SRCS cmake_utilitysourceast_test.cpp )

kde4_add_unit_test(cmake-utilitysourceasttest  ${utilitysourceasttest_SRCS} )
target_link_libraries( cmake-utilitysourceasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( variablerequiresasttest_SRCS cmake_variablerequiresast_test.cpp )

kde4_add_unit_test(cmake-variablerequiresasttest  ${variablerequiresasttest_SRCS} )
target_link_libraries( cmake-variablerequiresasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( vtkmakeinstantiatorasttest_SRCS cmake_vtkmakeinstantiatorast_test.cpp )

kde4_add_unit_test(cmake-vtkmakeinstantiatorasttest  ${vtkmakeinstantiatorasttest_SRCS} )
target_link_libraries( cmake-vtkmakeinstantiatorasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( vtkwrapjavaasttest_SRCS cmake_vtkwrapjavaast_test.cpp )

kde4_add_unit_test(cmake-vtkwrapjavaasttest  ${vtkwrapjavaasttest_SRCS} )
target_link_libraries( cmake-vtkwrapjavaasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( vtkwrappythonasttest_SRCS cmake_vtkwrappythonast_test.cpp )

kde4_add_unit_test(cmake-vtkwrappythonasttest  ${vtkwrappythonasttest_SRCS} )
target_link_libraries( cmake-vtkwrappythonasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( vtkwraptclasttest_SRCS cmake_vtkwraptclast_test.cpp )

kde4_add_unit_test(cmake-vtkwraptclasttest  ${vtkwraptclasttest_SRCS} )
target_link_libraries( cmake-vtkwraptclasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( whileasttest_SRCS cmake_whileast_test.cpp )

kde4_add_unit_test(cmake-whileasttest  ${whileasttest_SRCS} )
target_link_libraries( cmake-whileasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

set ( writefileasttest_SRCS cmake_writefileast_test.cpp )

kde4_add_unit_test(cmake-writefileasttest  ${writefileasttest_SRCS} )
target_link_libraries( cmake-writefileasttest
  ${QT_QTTEST_LIBRARY}
  ${KDE_KDECORE_LIBS}
  kdevcmakecommon )

#disable this for now, apol said its not working anyway
#set ( cmakeprojectvisitor_SRCS ../parser/cmakeprojectvisitor.cpp cmake_cmakeprojectvisitor_test.cpp )

#kde4_add_unit_test(cmake-cmakeprojectvisitor  ${cmakeprojectvisitor_SRCS} )
#target_link_libraries( cmake-cmakeprojectvisitor
#	 ${QT_QTTEST_LIBRARY}
#	 ${KDE_KDECORE_LIBS}
#	 kdevcmakecommon )

