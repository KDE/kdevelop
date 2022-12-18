/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    CTestTestfile.cmake parsing uses code from the xUnit plugin
    SPDX-FileCopyrightText: 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
    SPDX-FileCopyrightText: 2010 Daniel Calviño Sánchez <danxuliu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ctestutils.h"
#include "ctestsuite.h"
#include "ctestfindjob.h"
#include <debug_testing.h>

#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <QDir>

using namespace KDevelop;

static CMakeTarget targetByName(const QHash< KDevelop::Path, QVector<CMakeTarget>>& targets, const QString& name)
{
    for (const auto& subdir: targets) {
        for (const auto &target: subdir) {
            if (target.name == name)
                return target;
        }
    }

    return {};
}

static CMakeTarget targetByExe(const QHash< KDevelop::Path, QVector<CMakeTarget>>& targets, const KDevelop::Path& exe)
{
    for (const auto& subdir: targets) {
        for (const auto &target: subdir) {
            if (target.artifacts.contains(exe))
                return target;
        }
    }

    return {};
}

std::vector<std::unique_ptr<CTestSuite>>
CTestUtils::createTestSuites(const QVector<CMakeTest>& testSuites,
                             const QHash<KDevelop::Path, QVector<CMakeTarget>>& targets, KDevelop::IProject* project)
{
    std::vector<std::unique_ptr<CTestSuite>> suites;
    suites.reserve(testSuites.size());
    for (const CMakeTest& test : testSuites) {
        KDevelop::Path executablePath;
        CMakeTarget target;

        if (QDir::isAbsolutePath(test.executable)) {
            executablePath = KDevelop::Path(test.executable);
            target = targetByExe(targets, executablePath);
        } else {
            target = targetByName(targets, test.executable);
            if (target.artifacts.isEmpty()) {
                continue;
            }
            executablePath = target.artifacts.first();
        }

        qCDebug(CMAKE_TESTING) << "looking for tests in test" << test.name << "target" << target.name << "with sources"
                               << target.sources;

        suites.push_back(std::make_unique<CTestSuite>(test.name, executablePath, target.sources.toList(), project,
                                                      test.arguments, test.properties));
    }
    return suites;
}
