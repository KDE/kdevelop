# KDevelop

This repository contains the source code for the KDevelop IDE.
It also includes KDevelop Platform (kdevplatform) and most of the plugins.

Other KDevelop plugins can be found in [KDevelop Group](https://invent.kde.org/kdevelop) of KDE GitLab instance.

## User Documentation

User documentation is available from:
https://userbase.kde.org/KDevelop5/Manual

## Compile

KDevelop is built the same way as most KDE projects, using CMake to set up a build directory and build options.
For detailed instructions how to compile KDevelop, please refer to the Wiki:
https://community.kde.org/KDevelop/HowToCompile_v5 .

### Optional dependencies

Most of KDevelop's optional dependencies are opportunistic under the assumption that the build should
always use all available functionality, as well as the most recent version. CMake has a lesser known feature
to control which of such dependencies is used.

For instance, to skip building the Subversion plugin, use
`-DCMAKE_DISABLE_FIND_PACKAGE_SubversionLibrary=ON`.

## Contribute

If you want to contribute to KDevelop, please read through:
https://www.kdevelop.org/contribute-kdevelop

## Development Infrastructure
- [Bug tracker](https://bugs.kde.org/buglist.cgi?bug_status=UNCONFIRMED&bug_status=CONFIRMED&bug_status=ASSIGNED&bug_status=REOPENED&list_id=1777266&product=kdevelop&product=kdevplatform&query_format=advanced)
- [KDE GitLab instance (code review as well as hosting and other important collaboration tasks)](https://invent.kde.org/kdevelop/kdevelop/-/merge_requests)
- [Phabricator (task tracker until this functionality is migrated to GitLab)](https://phabricator.kde.org/dashboard/view/8/?)
