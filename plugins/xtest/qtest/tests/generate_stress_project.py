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


def generate_root_cmakelists(suites):
    f = open("CMakeLists.txt", 'w')
    f.write(\
'''
project(QTestStressTest)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
enable_testing()
''')
    for suite in suites:
        f.write("add_subdirectory(" + suite + ")\n")
    f.write("\n")
    f.close()

def generate_cmakelists_qt():
    f = open("CMakeLists.txt", 'w')
    f.write(\
'''
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

''')
    f.close()

def append_build_rule_qt(name):
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

def generate_testcase_qt(name):
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

def generate_cmakelists_kde():
    f = open("CMakeLists.txt", 'w')
    f.write(\
'''
find_package(KDE4 REQUIRED)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} ${KDE4_INCLUDES}})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

''')
    f.close()

def append_build_rule_kde(name):
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

def generate_testcase_kde(name):
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

def append_build_rule_kde_gui(name):
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

def generate_testcase_kde_gui(name):
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

def generate_testcase_with_body(name, body):
    f = open(name + ".h", 'w')
    f.write(\
'''
#include <qtest_kde.h>
#include <KDebug>
#ifndef ''' + name + '''_H
#define ''' + name + '''_H
class ''' + name + ''' : public QObject
{ Q_OBJECT
private slots:''')
    f.write(body)
    f.write(\
'''};
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

def generate_testcase_commands(name, count):
    body = ""
    for i in range(1, count):
        body += '    void foo%(#)04d() {}\n' %{"#":i}
    generate_testcase_with_body(name, body+"\n")

def generate_testcase_spam(name, count):
    body = '''void foo() {
'''
    for i in range(1, count):
        body += '        kDebug() << "%(#)04d";\n' %{"#":i}
    body += '''
    }'''
    generate_testcase_with_body(name, body)

def generate_testcase_asserts(name, count):
    body = '''void foo() {
'''
    for i in range(1, count, 3):
        body +=\
'''        QCOMPARE(1,1);
           QVERIFY(1);
           QVERIFY2(1, "failure message");
'''
    body += "QCOMPARE(1,0);}\n"
    generate_testcase_with_body(name, body)

def construct_name(base, count):
    return '%(b)s_%(#)04d' % {"b" : base, "#":count}

def generate_suite(name, generate_cmakelists, generate_testcase, count, append_build_rule):
    mkdir("suite_" + name)
    chdir("suite_" + name)
    generate_cmakelists()
    for i in range(1,count):
        case_name = construct_name('case_' + name, i)
        generate_testcase(case_name)
        append_build_rule(case_name)
        print "# Generated " + case_name
    chdir("..")
    last_case.append(construct_name('case_' + name, count-1))

#################### ENTRY POINT #########################

from time import time

before = time()

root = getcwd()
system("rm -rf stress") # ;-)
mkdir("stress")
chdir("stress")
generate_kdev4_files()
suites = ["suite_qt", "suite_kde_core", "suite_kde_gui", "suite_spam", "suite_asserts", "suite_commands"]
generate_root_cmakelists(suites)
last_case = []

generate_suite('qt',\
        generate_cmakelists_qt,\
        generate_testcase_qt, 250, \
        append_build_rule_qt)

generate_suite('kde_core', \
        generate_cmakelists_kde, \
        generate_testcase_kde, 250, \
        append_build_rule_kde)

generate_suite('kde_gui', \
        generate_cmakelists_kde, \
        generate_testcase_kde_gui, 250, \
        append_build_rule_kde_gui)

generate_suite('spam', \
        generate_cmakelists_kde, \
        lambda x: generate_testcase_spam(x, 50), 250, \
        append_build_rule_kde)

generate_suite('asserts', \
        generate_cmakelists_kde, \
        lambda x: generate_testcase_asserts(x,50), 250, \
        append_build_rule_kde)

mkdir("suite_commands")
chdir("suite_commands")
generate_cmakelists_kde()
append_build_rule_kde("case_commands")
generate_testcase_commands("case_commands", 250)
print "# Generated case_commands"
chdir("..")
last_case.append('case_commands')

chdir(root + "/stress")
mkdir("build")
chdir("build")

cmake = popen("cmake .. -DKDE4_BUILD_TESTS=on")
print "# Running cmake"
cmake.close()

before = time()
make = popen("make -j2")
build_times = []
last_case_ = last_case[:]
while 1:
    line = make.readline()
    if line == "": break
    #print line,
    if line.find("Built target") != -1:
        print "# " + line[line.find("Built target"):],
    if len(last_case_) != 0 and line.find("Built target " + last_case_[0]) != -1:
        build_times.append(time())
        last_case_ = last_case_[1:]

after = time()
print '# Total elapsed: %(#)d sec' % {"#":after-before}
prev = before
for i in range(0, len(last_case)):
    built_at = build_times[i]
    print '## Built time for %(c)s: %(#)d sec' % {"c":last_case[i], "#":built_at-prev}
    prev = built_at
