# kdevelop will crash when you comment/decomment the line below
# currently I get:
# ASSERT: "it" in file /home/nix/KdeDev/kdevelop/buildtools/managers/cmake/cmakemanager.cpp, line 690
# some time ago this would expose, https://bugs.kde.org/show_bug.cgi?id=168229

add_executable(foo foo.cpp)

