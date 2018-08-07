# kdev-clang-tidy

A plugin for [KDevelop](https://www.kdevelop.org) to support [Clang-Tidy](http://clang.llvm.org/extra/clang-tidy/) 
static analysis.
It uses the [KDevelop5 Problem Checker Framework](https://techbase.kde.org/KDevelop5/Problem_Checker_Framework), parsing 
clang-tidy's output and showing the issues on the Problems Viewer, allowing easy review of the code.

## Build and install
In order to build this plugin you must have the libraries for KDevPlatform and KDevelop installed into your system.
Get the source code from this repository (see https://commits.kde.org/kdev-clang-tidy ), tweak it if you want, run 
cmake to configure the project, build it, install it and have fun.
