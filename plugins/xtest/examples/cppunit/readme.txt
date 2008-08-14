A tiny examplary CppUnit test suite. To run it you will need

*/ CppUnit
*/ KDevelop4
*/ KDevelop4 cppunit main wrapper, ie `cppunitwrapper.h' with the
   CPPUNIT_XTEST_MAIN macro. This should be located in
   kdevelop/veritas/ on your KDE4 include path.

Steps required [will get user-friendlier]:

*/ Run Kdevelop4
*/ Open project 'cppunit.kdev4'
*/ Build, by right click -> build item(s) in the project
   toolview
*/ Configure cppunit toolview:
      Project -> Configure -> CppUnit -> point to the moneytest
      executable
*/ Open CppUnit toolview:
      View -> Add Tool View -> CppUnit Runner

Now the Money test case should be visible & runnable. The
`testZero' command is supposed to fail.
