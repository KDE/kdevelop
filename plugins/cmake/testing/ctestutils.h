/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CTESTUTILS_H
#define CTESTUTILS_H

#include "cmakeprojectdata.h"

#include <memory>
#include <vector>

namespace KDevelop
{
class IProject;
}

class CTestSuite;

namespace CTestUtils
{
std::vector<std::unique_ptr<CTestSuite>> createTestSuites(const QVector<CMakeTest>& testSuites,
                                                          const QHash<KDevelop::Path, QVector<CMakeTarget>>& targets,
                                                          KDevelop::IProject* project);
}

#endif // CTESTUTILS_H
