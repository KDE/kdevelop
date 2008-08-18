A tiny examplary QTest test suite. To run it you will need

*/ QTestLib
*/ KDevelop4

Steps required:

*/ Run Kdevelop4
*/ Open project 'qtest.kdev4'
*/ Build with cmake, be sure to enable KDE4_BUILD_TESTS under
   Project -> Configure -> CMake -> Show Advanced
*/ Open QTest toolview:
      View -> Add Tool View -> QTest Runner
*/ Select qtest project in the dropdown box and hit reload.
*/ Retrieve further instructions with 'what's this?'

The Money test case should be visible & runnable. The
'zero' command is supposed to fail.

note: suite.xml has become obsolete.