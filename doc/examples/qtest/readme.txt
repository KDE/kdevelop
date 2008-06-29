A tiny examplary QTest test suite. To run it you will need

*/ QTestLib
*/ KDevelop4

Steps required [will get user-friendlier]:

*/ Run Kdevelop4
*/ Open project 'qtest.kdev4'
*/ Build with cmake, be sure to enable KDE4_BUILD_TESTS under
   Project -> Configure -> CMake -> Show Advanced
*/ Configure qtest toolview:
      Project -> Configure -> QTest -> point to suite.xml
*/ Open QTest toolview:
      View -> Add Tool View -> QTest Runner
*/ Select qtest project in the dropdown box and hit reload.

The Money test case should be visible & runnable. The
'zero' command is supposed to fail.
