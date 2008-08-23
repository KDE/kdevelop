#!/usr/bin/python

from os import chdir, mkdir, system, getcwd, popen

#################### FUNCTIONS ###########################

KDEVELOP_ROOT="/home/nix/KdeDev/kdevelop"

def generate_kdev4_files():
    f = open("QTestStressTest.kdev4", 'w')
    f.write(\
'''[Project]
Name=QTestStressTest
Manager=KDevCMakeManager
''')
    f.close()
#    mkdir(".kdev4")
#    chdir(".kdev4")
#    f = open("QTestStressTest.kdev4", 'w')
#    f.write(\
'''
[CMake]
BuildDirs[$e]=''' + KDEVELOP_ROOT + '''/plugins/xtest/qtest/tests/stress/build/
CMakeDir=/usr/share/cmake/Modules
Current CMake Binary=file:///usr/bin/cmake
CurrentBuildDir=file:// ''' + KDEVELOP_ROOT + '''/plugins/xtest/qtest/tests/stress/build/
CurrentInstallDir=file:///usr/local
'''#)
#    f.close()
#    chdir("..")


def generate_root_cmakelists():
    f = open("CMakeLists.txt", 'w')
    f.write(\
'''
project(QTestStressTest)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
enable_testing()

add_subdirectory(suite_qt)
add_subdirectory(suite_kde)
add_subdirectory(suite_kde_gui)
''')
    f.close()

def qt_cmake():
    f = open("CMakeLists.txt", 'w')
    f.write(\
'''
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

''')
    f.close()

def qt_build_rule(name):
    f = open("CMakeLists.txt", 'a')
    cpp = name + ".cpp"
    moc = name + ".moc"
    target = name
    f.write(\
'''
qt4_generate_moc(''' + cpp + " " + moc + ''')
add_executable(''' + target + " " + cpp + " ${CMAKE_CURRENT_BINARY_DIR}/" + moc +''')
add_test(''' + target + " " + target + ''')
target_link_libraries(''' + target + ''' ${QT_QTTEST_LIBRARY})

''')
    f.close()

def qt_case_cpp(name):
    f = open(name + ".cpp", 'w')
    f.write(\
'''
#include <QtCore/QObject>
#include <QtTest/QTest>

class ''' + name + ''' : public QObject
{ Q_OBJECT
private slots:
    void foo() {}
};

#include "''' + name + '''.moc"
QTEST_MAIN(''' + name + ''')
''')
    f.close()

def kde_cmake():
    f = open("CMakeLists.txt", 'w')
    f.write(\
'''
find_package(KDE4 REQUIRED)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} ${KDE4_INCLUDES}})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

''')
    f.close()

def kde_build_rule(name):
    f = open("CMakeLists.txt", 'a')
    cpp = name + ".cpp"
    moc = name + ".moc"
    target = name
    f.write(\
'''
kde4_add_unit_test(''' + target + " " + cpp + " ${CMAKE_CURRENT_BINARY_DIR}/" + moc + ''')
target_link_libraries(''' + target + ''' ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

''')
    f.close()

def kde_case(name):
    f = open(name + ".h", 'w')
    f.write(\
'''
#include <qtest_kde.h>
#ifndef ''' + name + '''_H
#define ''' + name + '''_H
class ''' + name + ''' : public QObject
{ Q_OBJECT
private slots:
    void foo() {}
};
#endif
''');
    f.close()
    f = open(name + ".cpp", 'w')
    f.write(\
'''#include "''' + name + '''.h"
#include "''' + name + '''.moc"
QTEST_KDEMAIN_CORE(''' + name + ''')
''')
    f.close()

def kde_gui_build_rule(name):
    f = open("CMakeLists.txt", 'a')
    cpp = name + ".cpp"
    moc = name + ".moc"
    target = name
    f.write(\
'''
kde4_add_unit_test(''' + target + " " + cpp + " ${CMAKE_CURRENT_BINARY_DIR}/" + moc + ''')
target_link_libraries(''' + target + ''' ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

''')
    f.close()

def kde_gui_case(name):
    f = open(name + ".h", 'w')
    f.write(\
'''
#include <qtest_kde.h>
#ifndef ''' + name + '''_H
#define ''' + name + '''_H
class ''' + name + ''' : public QObject
{ Q_OBJECT
private slots:
    void foo() {}
};
#endif
''');
    f.close()
    f = open(name + ".cpp", 'w')
    f.write(\
'''#include "''' + name + '''.h"
#include "''' + name + '''.moc"
QTEST_KDEMAIN(''' + name + ''', GUI)
''')
    f.close()

def generate_suite(name, count, cmake_gen, app_build_rule, case_cpp_gen):
    mkdir("suite_" + name)
    chdir("suite_" + name)
    cmake_gen()
    for i in range(1,count):
        case_name = 'case_%(n)s_%(#)04d' % {"n":name, "#":i}
        case_cpp_gen(case_name)
        app_build_rule(case_name)
        print "# Generated " + case_name
    chdir("..")

#################### ENTRY POINT #########################

from time import time

before = time()

root = getcwd()
system("rm -rf stress") # ;-)
mkdir("stress")
chdir("stress")
generate_kdev4_files()
generate_root_cmakelists()

generate_suite("qt",  250, qt_cmake, qt_build_rule, qt_case_cpp)
generate_suite("kde", 250, kde_cmake, kde_build_rule, kde_case)
generate_suite("kde_gui", 250, kde_cmake, kde_gui_build_rule, kde_gui_case)

chdir(root + "/stress")
mkdir("build")
chdir("build")
make = popen("cmake .. -DKDE4_BUILD_TESTS=on && make -j2")
while 1:
    line = make.readline()
    if line == "": break
    #print line,
    if line.find("Built target") != -1:
        print "# " + line[line.find("Built target"):],

after = time()
print '# Total elapsed: %(#)d sec' % {"#":after-before}
