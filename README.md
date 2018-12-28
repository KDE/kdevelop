# KDevelop

This repository contains the source code for the KDevelop IDE.

The idea that this repository contains
- Every plugin related to C/C++ development
- Every plugin only specific for KDevelop (e.g. the Welcome Page plugin)

## User Documentation

User documentation is available from:
https://userbase.kde.org/KDevelop4/Manual

## Compile

KDevelop is built the same way as most KDE projects, using CMake to set up a build directory and build options.
For detailed instructions how to compile KDevelop, please refer to the Wiki:
https://community.kde.org/KDevelop/HowToCompile_v5 .

### Optional dependencies

Most of KDevelop's optional dependencies are opportunistic under the assumption that the build should
always use all available functionality, as well as the most recent version. CMake has a lesser known feature
to control which of such dependencies is used.

For instance, to skip building the Subversion plugin, use
`-DCMAKE_DISABLE_FIND_PACKAGE_SubversionLibrary=ON`. To use QtWebKit instead of QtWebEngine for
documentation rendering when both are available, add `-DCMAKE_DISABLE_FIND_PACKAGE_Qt5WebEngineWidgets=ON` to
the CMake arguments.

## Contribute

If you want to contribute to KDevelop, please read through:
https://www.kdevelop.org/contribute-kdevelop

## Development Infrastructure
- [Bug tracker](https://bugs.kde.org/buglist.cgi?bug_status=UNCONFIRMED&bug_status=CONFIRMED&bug_status=ASSIGNED&bug_status=REOPENED&list_id=1408918&product=kdevelop&query_format=advanced)
- [Phabricator (task tracker, code review and more)](https://phabricator.kde.org/dashboard/view/8/?)
