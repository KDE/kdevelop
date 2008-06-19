/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CPPUNIT_WRAPPER_H
#define CPPUNIT_WRAPPER_H

#include <exception>
#include <cppunit/TestCase.h>
#include <cppunit/TestPath.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>

//
// CppUnit's terminology is a tad different from
// the standard way of naming things.
//
// The following mapping is used:
// CppUnit                           --> xTest
// TestCase                          --> TestCommand
// TestSuite (with case children)    --> TestCase
// TestSuite (without case children) --> TestSuite
//

namespace QxCppUnit
{

static inline bool isSuite(CppUnit::Test* test)
{
    CppUnit::TestSuite* suite = dynamic_cast<CppUnit::TestSuite*>(test);
    if (!suite || suite->getChildTestCount() == 0)
        return false;
    CppUnit::Test* child = suite->getChildTestAt(0);
    CppUnit::TestCase* caze = dynamic_cast<CppUnit::TestCase*>(child);
    return (caze == 0); // no testcase children, ie a pure suite
}

static inline bool isCase(CppUnit::Test* test)
{
    CppUnit::TestSuite* suite = dynamic_cast<CppUnit::TestSuite*>(test);
    if (!suite) {
        return false;
    }
    if (suite->getChildTestCount() == 0) {
        return true;
    }
    CppUnit::Test* child = suite->getChildTestAt(0);
    CppUnit::TestCase* caze = dynamic_cast<CppUnit::TestCase*>(child);
    return (caze != 0);
}

static inline bool isCommand(CppUnit::Test* test)
{
    CppUnit::TestCase* cmd = dynamic_cast<CppUnit::TestCase*>(test);
    return (cmd != 0);
}

static inline const char* tagFor(CppUnit::Test* test)
{
    static const char* suite = "suite";
    static const char* caze  = "case";
    static const char* cmd   = "command";
    static const char* unkw  = "unkown";
    const char* tag;
    if (isSuite(test)) {
        tag = suite;
    } else if (isCase(test)) {
        tag = caze;
    } else if (isCommand(test)) {
        tag = cmd;
    } else {
        tag = unkw;
    }
    return tag;
}

// recursivly print the test structure as XML
static inline void printTestTree(CppUnit::Test* test)
{
    const char* tag = tagFor(test);
    int nrofChilds = test->getChildTestCount();
    if (nrofChilds == 0) {
        printf("<%s name=\"%s\" />\n", tag, test->getName().c_str());
    } else {
        printf("<%s name=\"%s\" >\n",  tag, test->getName().c_str());
        for (int i = 0; i < nrofChilds; i++) {
            printTestTree(test->getChildTestAt(i));
        }
        printf("</%s>\n", tag);
    }
}

class XMLProgressListener : public CppUnit::TestListener
{
public:
    XMLProgressListener() {};
    virtual ~XMLProgressListener() {};

    virtual void startTest(CppUnit::Test *test) {
        printf("<command name=\"%s\">\n", test->getName().c_str());
    }

    virtual void addFailure(const CppUnit::TestFailure &failure) {
        CppUnit::SourceLine loc = failure.sourceLine();
        printf("<status type=\"%s\" file=\"%s\" line=\"%d\"/>\n",
               (failure.isError() ? "error" : "assertion"),
               loc.fileName().c_str(),
               loc.lineNumber());
    }

    virtual void endTest(CppUnit::Test*) {
        printf("</command>\n");
    }

    virtual void startSuite(CppUnit::Test* test) {
        printf("<%s name=\"%s\">\n", tagFor(test), test->getName().c_str());
    }

    virtual void endSuite(CppUnit::Test *test) {
        printf("</%s>\n", tagFor(test));
    }

private:
    XMLProgressListener(const XMLProgressListener &copy);
    void operator=(const XMLProgressListener &copy);

};

static inline void runTest(CppUnit::Test* root)
{
    CppUnit::TestResult testresult;
    QxCppUnit::XMLProgressListener progress;
    testresult.addListener(&progress);
    CppUnit::TestRunner testrunner;
    testrunner.addTest(root);
    testrunner.run(testresult);
}

CppUnit::Test* rootSuite;

static inline void runAllTests(CppUnit::Test* root, int argc, char **argv)
{
    printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
           "<root>\n");
    if (argc == 1) {
        // run evrything
        runTest(root);
    } else {
        try {
        for (int i = 1; i < argc; i++) {
            CppUnit::TestPath path;
            path = root->resolveTestPath(argv[i]);
            runTest(path.getChildTest());
        }
        } catch (std::exception& exc) {
            printf("<error message=\"%s\" />\n", exc.what());
        } catch (...) {
            printf("<error message=\"unkown exception occured in client test code\"/>\n");
        }
    }
    printf("</root>\n");
}

#define CPPUNIT_XTEST_MAIN( root ) \
int main(int argc, char** argv)\
{\
    if (argc == 1 || argv[1][0] != '-') { \
        QxCppUnit::runAllTests(root, argc, argv); \
    } else { \
        QxCppUnit::printTestTree(root); \
    }\
    return 0;\
}


} // namespace QxCppUnit

#endif // CPPUNIT_WRAPPER
