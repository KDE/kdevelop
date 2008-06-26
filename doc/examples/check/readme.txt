A tiny examplary Check test suite. To run it you will need

*/ Check
*/ KDevelop4
*/ KDevelop4 check main wrapper, ie `checkwrapper.h' with the
   CHECK_XTEST_MAIN macro.

Steps required [will get user-friendlier]:

*/ Run Kdevelop4
*/ Open project 'check.kdev4'
*/ Adapt the KDEV_CHECK variable in Makefile to the directory
   which contains `checkwrapper.h'
*/ Build, by right click -> build item(s) in the project
   toolview
*/ Configure check toolview:
      Project -> Configure Check -> Check -> point to check_money
*/ Open Check toolview:
      View -> Add Tool View -> Check Runner

Now the Money test case should be visible. The create_zero test
is expected to fail with a seg fault.
