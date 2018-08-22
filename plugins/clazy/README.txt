This plugin integrates Clazy to KDevelop.

Clazy is a compiler plugin which allows clang to understand Qt semantics. You get more than 50 Qt related compiler warnings, ranging from unneeded memory allocations to misusage of API, including fix-its for automatic refactoring.

https://commits.kde.org/clazy

The plugin allows you to check project's code with clazy checker. Runtime dependencies:

* clazy-standalone (clazy part)
* installed clazy docs (used to building checks DB with errors descriptions)
* make (analysis run, see later)
* compile_commands.json present in project's build directory

Plugin's GUI provides easy way to clazy configuration, enabling/disabling checks and so on.

Analysis run done through executing system make command with custom makefile which generates by the plugin in the project's build directory. Such approach allows us to enable/disable parallel code analysis with simple setup of standard make "-j" parameter. Current makefile structure assumes that each checked source file is independent from all others so we can efficiently load all present CPUs - scalability is close to linear.
