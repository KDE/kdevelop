CMAKE_MINIMUM_REQUIRED(VERSION 1.5)

INCLUDE_REGULAR_EXPRESSION("^.*$")

# configure the .h file
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmConfigure.cmake.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmConfigure.h"
  )
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CPack/cmCPackConfigure.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CPack/cmCPackConfigure.h"
  )

# add the include path to find the .h
INCLUDE_DIRECTORIES(
  "${CMake_BINARY_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source"
  ${CMAKE_ZLIB_INCLUDES}
  ${CMAKE_EXPAT_INCLUDES}
  ${CMAKE_TAR_INCLUDES}
  )

# let cmake know it is supposed to use it
ADD_DEFINITIONS(-DCMAKE_BUILD_WITH_CMAKE)

OPTION(CMAKE_REGENERATE_YACCLEX
  "Regenerate YACC and LEXX files" OFF)
MARK_AS_ADVANCED(CMAKE_REGENERATE_YACCLEX)
IF(CMAKE_REGENERATE_YACCLEX)
  SET(parsersLexers cmCommandArgument cmExpr)
  FIND_PROGRAM(YACC_EXECUTABLE
    NAMES yacc bison
    PATHS /usr/bin
    DOC "Yacc or Bison executable")
  FIND_PROGRAM(FLEX_EXECUTABLE
    NAMES flex
    PATHS /usr/bin
    DOC "Flex executable")
  MARK_AS_ADVANCED(YACC_EXECUTABLE FLEX_EXECUTABLE)
  IF(YACC_EXECUTABLE)
    SET(BISON_FLAGS)
    IF(YACC_EXECUTABLE MATCHES "bison")
      SET(BISON_FLAGS "--yacc")
    ENDIF(YACC_EXECUTABLE MATCHES "bison")
    SET(yacc_files)
    FOREACH(name ${parsersLexers})
      SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Parser.y")
      SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Parser.cxx")
      SET(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}ParserTokens.h")
      ADD_CUSTOM_COMMAND(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND ${YACC_EXECUTABLE}
        ARGS --name-prefix=${name}_yy --defines="${hdr}" -o"${dst}" "${src}")
      SET(yacc_files ${yacc_files} "${dst}")
    ENDFOREACH(name)
    ADD_CUSTOM_TARGET(RerunYacc DEPENDS ${yacc_files})
  ENDIF(YACC_EXECUTABLE)
  IF(FLEX_EXECUTABLE)
    SET(lex_files)
    FOREACH(name ${parsersLexers})
      SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Lexer.in.l")
      SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.cxx")
      SET(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.h")
      ADD_CUSTOM_COMMAND(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS --prefix=${name}_yy --header-file="${hdr}" -o"${dst}" "${src}")
      SET(lex_files ${lex_files} "${dst}")
    ENDFOREACH(name)
    ADD_CUSTOM_TARGET(RerunLex DEPENDS ${lex_files})
  ENDIF(FLEX_EXECUTABLE)

ENDIF(CMAKE_REGENERATE_YACCLEX)
#
# Sources for CMakeLib
#
SET(SRCS
  cmBootstrapCommands.cxx
  cmCacheManager.cxx
  cmCacheManager.h
  cmCommands.cxx
  cmCommands.h
  cmCommandArgumentLexer.cxx
  cmCommandArgumentParser.cxx
  cmCommandArgumentParserHelper.cxx
  cmCustomCommand.cxx
  cmCustomCommand.h
  cmDepends.cxx
  cmDepends.h
  cmDependsC.cxx
  cmDependsC.h
  cmDependsFortran.cxx
  cmDependsFortran.h
  cmDependsFortranLexer.cxx
  cmDependsFortranParser.cxx
  cmDependsFortranParser.h
  cmDependsJava.cxx
  cmDependsJava.h
  cmDependsJavaLexer.cxx
  cmDependsJavaParser.cxx
  cmDependsJavaParserHelper.cxx
  cmDependsJavaParserHelper.h
  cmDocumentation.cxx
  cmDynamicLoader.cxx
  cmDynamicLoader.h
  cmExprLexer.cxx
  cmExprParser.cxx
  cmExprParserHelper.cxx
  cmGeneratedFileStream.cxx
  cmGlobalGenerator.cxx
  cmGlobalGenerator.h
  cmGlobalUnixMakefileGenerator3.cxx
  cmGlobalUnixMakefileGenerator3.h
  cmInstallGenerator.h
  cmInstallGenerator.cxx
  cmInstallFilesGenerator.h
  cmInstallFilesGenerator.cxx
  cmInstallScriptGenerator.h
  cmInstallScriptGenerator.cxx
  cmInstallTargetGenerator.h
  cmInstallTargetGenerator.cxx
  cmListFileCache.cxx
  cmListFileCache.h
  cmListFileLexer.c
  cmLocalGenerator.cxx
  cmLocalGenerator.h
  cmLocalUnixMakefileGenerator3.cxx
  cmMakeDepend.cxx
  cmMakeDepend.h
  cmMakefile.cxx
  cmMakefile.h
  cmMakefileTargetGenerator.cxx
  cmMakefileExecutableTargetGenerator.cxx
  cmMakefileLibraryTargetGenerator.cxx
  cmMakefileUtilityTargetGenerator.cxx
  cmOrderLinkDirectories.cxx
  cmFileTimeComparison.cxx
  cmFileTimeComparison.h
  cmSourceFile.cxx
  cmSourceFile.h
  cmSourceGroup.cxx
  cmSourceGroup.h
  cmSystemTools.cxx
  cmSystemTools.h
  cmTarget.cxx
  cmTarget.h
  cmTest.cxx
  cmTest.h
  cmVariableWatch.cxx
  cmVariableWatch.h
  cmVersion.cxx
  cmVersion.h
  cmXMLParser.cxx
  cmXMLParser.h
  cmake.cxx
  cmake.h
  cmakewizard.cxx
  cmakewizard.h
  )

# Kdevelop only works on UNIX and not windows
IF(UNIX)
  SET(SRCS ${SRCS}
    cmGlobalKdevelopGenerator.cxx
    cmLocalKdevelopGenerator.cxx)
ENDIF(UNIX)
# XCode only works on apple
IF(APPLE)
  SET(SRCS ${SRCS}
    cmXCodeObject.cxx
    cmXCode21Object.cxx
    cmGlobalXCodeGenerator.cxx
    cmGlobalXCodeGenerator.h
    cmGlobalXCode21Generator.cxx
    cmGlobalXCode21Generator.h
    cmLocalXCodeGenerator.cxx
    cmLocalXCodeGenerator.h)
ENDIF(APPLE)


IF (WIN32)
  IF(NOT UNIX)
    SET(SRCS ${SRCS}
      cmGlobalBorlandMakefileGenerator.cxx
      cmGlobalBorlandMakefileGenerator.h
      cmGlobalMSYSMakefileGenerator.cxx
      cmGlobalMinGWMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.h
      cmGlobalVisualStudio6Generator.cxx
      cmGlobalVisualStudio6Generator.h
      cmGlobalVisualStudio71Generator.cxx
      cmGlobalVisualStudio71Generator.h
      cmGlobalVisualStudio7Generator.cxx
      cmGlobalVisualStudio7Generator.h
      cmGlobalVisualStudio8Generator.cxx
      cmGlobalVisualStudio8Generator.h
      cmGlobalVisualStudio8Win64Generator.cxx
      cmGlobalVisualStudio8Win64Generator.h
      cmGlobalWatcomWMakeGenerator.cxx
      cmLocalVisualStudio6Generator.cxx
      cmLocalVisualStudio6Generator.h
      cmLocalVisualStudio7Generator.cxx
      cmLocalVisualStudio7Generator.h
      cmWin32ProcessExecution.cxx
      cmWin32ProcessExecution.h
      )
  ENDIF(NOT UNIX)
ENDIF (WIN32)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})
TARGET_LINK_LIBRARIES(CMakeLib cmsys ${CMAKE_EXPAT_LIBRARIES} ${CMAKE_ZLIB_LIBRARIES} ${CMAKE_TAR_LIBRARIES})

# On Apple we need Carbon
IF(APPLE)
  TARGET_LINK_LIBRARIES(CMakeLib "-framework Carbon")
ENDIF(APPLE)

# On some platforms we need the rpcrt4 library for the VS 7 generators.
IF(CMAKE_BUILD_ON_VISUAL_STUDIO OR MINGW)
  TARGET_LINK_LIBRARIES(CMakeLib rpcrt4)
ENDIF(CMAKE_BUILD_ON_VISUAL_STUDIO OR MINGW)

#
# CTestLib
#
INCLUDE_DIRECTORIES(
  "${CMAKE_SOURCE_DIR}/Source/CTest"
  ${CMAKE_XMLRPC_INCLUDES}
  ${CMAKE_CURL_INCLUDES}
  )
#
# Sources for CTestLib
#
SET(CMTEST_SRCS cmCTest.cxx
  CTest/cmCTestBuildAndTestHandler.cxx
  CTest/cmCTestBuildCommand.cxx
  CTest/cmCTestBuildHandler.cxx
  CTest/cmCTestConfigureCommand.cxx
  CTest/cmCTestConfigureHandler.cxx
  CTest/cmCTestCoverageCommand.cxx
  CTest/cmCTestCoverageHandler.cxx
  CTest/cmCTestEmptyBinaryDirectoryCommand.cxx
  CTest/cmCTestGenericHandler.cxx
  CTest/cmCTestHandlerCommand.cxx
  CTest/cmCTestMemCheckCommand.cxx
  CTest/cmCTestMemCheckHandler.cxx
  CTest/cmCTestReadCustomFilesCommand.cxx
  CTest/cmCTestRunScriptCommand.cxx
  CTest/cmCTestScriptHandler.cxx
  CTest/cmCTestSleepCommand.cxx
  CTest/cmCTestStartCommand.cxx
  CTest/cmCTestSubmitCommand.cxx
  CTest/cmCTestSubmitHandler.cxx
  CTest/cmCTestTestCommand.cxx
  CTest/cmCTestTestHandler.cxx
  CTest/cmCTestUpdateCommand.cxx
  CTest/cmCTestUpdateHandler.cxx
  )

# Build CTestLib
#ADD_LIBRARY(CTestLib ${CMTEST_SRCS})
#TARGET_LINK_LIBRARIES(CTestLib CMakeLib ${CMAKE_CURL_LIBRARIES} ${CMAKE_XMLRPC_LIBRARIES})

#
# Sources for CPack
#
SET(CPACK_SRCS
  CPack/cmCPackGenerators.cxx
  CPack/cmCPackSTGZGenerator.cxx
  CPack/cmCPackTGZGenerator.cxx
  CPack/cmCPackNSISGenerator.cxx
  CPack/cmCPackPackageMakerGenerator.cxx
  CPack/cmCPackZIPGenerator.cxx
  CPack/cmCPackTarBZip2Generator.cxx
  CPack/cmCPackTarCompressGenerator.cxx
  CPack/cmCPackGenericGenerator.cxx
  CPack/cmCPackLog.cxx
  )
# Build CPackLib
#ADD_LIBRARY(CPackLib ${CPACK_SRCS})
#TARGET_LINK_LIBRARIES(CPackLib CMakeLib)

INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
