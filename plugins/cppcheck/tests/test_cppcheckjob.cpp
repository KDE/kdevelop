/*************************************************************************************
 *  Copyright 2016 (C) Anton Anikin <anton.anikin@htower.ru>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "test_cppcheckjob.h"

#include <QTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "job.h"
#include "parameters.h"

using namespace KDevelop;
using namespace cppcheck;

class JobTester : public Job
{
    Q_OBJECT

public:
    explicit JobTester(const Parameters& params) : Job(params) {}

    using Job::postProcessStdout;
    using Job::postProcessStderr;

    QString standardOutput() const { return m_standardOutput.join('\n'); }
    QString xmlOutput() const { return m_xmlOutput.join('\n'); }
};

void TestCppcheckJob::initTestCase()
{
    AutoTestShell::init({"kdevcppcheck"});
    TestCore::initialize(Core::NoUi);
}

void TestCppcheckJob::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCppcheckJob::testJob()
{
    QStringList stdoutOutput = {
        "Checking source1.cpp...",
        "1/2 files checked 50% done",
        "Checking source2.cpp...",
        "2/2 files checked 50% done"
    };

    QStringList stderrOutput = {
        "(information) Couldn't find path given by -I '/missing_include_dir_1/'",
        "(information) Couldn't find path given by -I '/missing_include_dir_2/'",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
        "<results version=\"2\">",
        "   <cppcheck version=\"1.72\"/>",
        "   <errors>",
        "       <error id=\"missingInclude\" severity=\"information\" msg=\"msg...\" verbose=\"verbose...\"/>",
        "   </errors>",
        "</results>"
    };

    Parameters jobParams;
    JobTester jobTester(jobParams);

    jobTester.postProcessStderr(stderrOutput);
    jobTester.postProcessStdout(stdoutOutput);

    // move non-XML elements from stderrOutput
    stdoutOutput.push_front(stderrOutput[1]);
    stdoutOutput.push_front(stderrOutput[0]);
    stderrOutput.pop_front();
    stderrOutput.pop_front();

    QCOMPARE(jobTester.standardOutput(), stdoutOutput.join('\n'));
    QCOMPARE(jobTester.xmlOutput(), stderrOutput.join('\n'));
}

QTEST_GUILESS_MAIN(TestCppcheckJob)

#include "test_cppcheckjob.moc"
