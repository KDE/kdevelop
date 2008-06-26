A tiny examplary CppUnit test suite. To run it you will need

*/ CppUnit
*/ KDevelop4
*/ KDevelop4 cppunit main wrapper, ie `cppunitwrapper.h' with the
   CPPUNIT_XTEST_MAIN macro.

Steps required [will get user-friendlier]:

*/ Run Kdevelop4
*/ Open project 'cppunit.kdev4'
*/ Adapt the KDEV_CPPUNIT variable in Makefile to the directory
   which contains `cppunitwrapper.h'
*/ Build, by right click -> build item(s) in the project
   toolview
*/ Configure check toolview:
      Project -> Configure Check -> Check -> point to check_money
*/ Open Check toolview:
      View -> Add Tool View -> Check Runner

Now the Money test case should be visible.
