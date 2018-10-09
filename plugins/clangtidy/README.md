# kdev-clang-tidy

A plugin for [KDevelop](https://www.kdevelop.org) to support [Clang-Tidy](http://clang.llvm.org/extra/clang-tidy/) static analysis.
It uses the [KDevelop5 Problem Checker Framework](https://techbase.kde.org/KDevelop5/Problem_Checker_Framework), parsing clang-tidy's output and showing the issues on the Problems Viewer, allowing easy review of the code.

The plugin provides some very basic features for the start:

* running clang-tidy on a single file, subdirectories or the whole project,
  with the result shown in the Problems tool view,
  invokable from file context menus and main menu (Analyzer section)
* per-project config UI:
  * free field for additional command line parameters to clang-tidy
  * checkbox whether to include system headers in the check
  * toggle between using .clang-tidy files or options in the config UI:
    * header filter regular expression
    * opt-in list of checks

See below what might be coming next.

## Get It

Either build it yourself from the sources (see below).

Or search the package manager of your Linux distribution or *BSD system for it (e.g. kdevelop-clang-tidy or kdev-clang-tidy):

* Arch Linux: kdevelop-clang-tidy
* Gentoo: kdevelop-clang-tidy
* openSUSE: kdevelop5-plugin-clang-tidy
* (please report the name in your package system/distribution, so it can be listed here).

Or try to install it via its [AppStream Id Link](appstream://org.kde.kdev-clang-tidy)


## Use It

Once installed, open the global KDevelop settings: ensure in the "Plugins" page that the "Clang-Tidy Support" plugin is enabled, then in the "Analyzers"/"Clang-Tidy" page make sure the clang-tidy executable path points to an existing executable.

Next open the configuration dialog for a project and check the settings in the "Clang-Tidy" page.

Then open a C++ source file of your project. Now invoke a run of clang-tidy either via the main menu "Code"/"Analyze Current File With"/"Analyze Current File With Clang-Tidy" or via the context menu on the file "Analyze Current File With"/"Clang-Tidy". A job should be started and, once finished, the "Problems" tool view should show up with the page "Clang-Tidy" selected, listing the result of the run.

**NOTE:**
For now one has to enable oneself the creation of the so-called compilation database which clang-tidy uses, e.g. with CMake to set the flag         `CMAKE_EXPORT_COMPILE_COMMANDS` to `ON`
in the settings (listed under "Advanced values" in the project's CMake settings in KDevelop). The plugin does not (yet) handle that for you.


## Report Bugs & Feature Requests

Tell what issues you have or would like to see at the [KDE bug tracker, product "KDevelop", component "Analyzer: Clang-Tidy"](https://bugs.kde.org/enter_bug.cgi?format=guided&amp;product=kdevelop&amp;component=Analyzer:%20Clang-Tidy)


## Package and Distribute It

The sources of this plugin build against the current stable KDevelop version, 5.2 (and also master). The result will only work with the version built against (5.2 or master).

Download tarballs of released versions from https://download.kde.org/stable/kdev-clang-tidy

Best create packages like you do from the tarballs of other separate KDevelop plugins, like kdev-python & kdev-php (which are released in the official KDevelop release bundle).

See also notes about building below.

## Build and Install It

Building the sources is done with CMake as build confguration tool, the usual things apply.

Requirement for building are the development files/package of KDevPlatform, at least version 5.2.

To have the KDevelop application see the new plugin, there are multiple options:

* the plugin either needs to be installed to the installation prefix where kdevelop is installed (e.g. using CMake flag `-DCMAKE_INSTALL_PREFIX="/usr"` with a kdevelop as installed from packages typically).
* the kdevelop instances need to be informed about the custom plugin install directory by seeing the environment variable `QT_PLUGIN_PATH` which is set to also includes the base path to the plugin, i.e. from where `kdevplatform/<number>/kdevclangtidy.so` is found (e.g. having in the `.bashrc` file a line `export QT_PLUGIN_PATH=$HOME/opt/kdevelop/lib64/plugins:$QT_PLUGIN_PATH` if the CMake flag `-DCMAKE_INSTALL_PREFIX=$HOME/opt/kdevelop` was used and if `lib64` is the system's library subdir name).

**NOTE:**
Set the CMake flag -DBUILD_TESTING=OFF if kdevelop has been built with that flag value.


## Learn What is on the Development Road

Planned features (in rough order of execution):

* warning about missing compilation database
* support for fix-its
* globally defined sets of checks, for reuse between projects and use for
  specific code analyze work
* folder-specific clang-tidy settings
* being an editor for .clang-tidy files, for seamless integration between
  IDE-based and console-based usage of clang-tidy
* enabling the generation of compilation database by simple button when needed

Long term the kdev-clang-tidy plugin should be added to the kdevelop code repository, as planned before (cmp. [KDevelop 5.1 release notes](https://www.kdevelop.org/news/kdevelop-510-released) ).

Even better though would be somebody implementing integration of this and other clang-based analyzers directly with the clang-parser of the KDevelop clang-based language plugin, so any warnings and fix-its are delivered as part of the normal editing. Though this might be more challenging, so the current explicit analyzer plugins serve as intermediate utilities.

## Get in Contact

This plugin is currently developed by KDevelop contributors, who can be reached via the usual [KDevelop support channels](https://www.kdevelop.org/support).
