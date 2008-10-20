/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

#ifndef CMAKECMAKEPROJECTVISITORTEST_H
#define CMAKECMAKEPROJECTVISITORTEST_H

#include <QtTest/QtTest>
#include <QString>
#include <language/duchain/topducontext.h>
#include "cmakelistsparser.h"

class CMakeProjectVisitor;

class CMakeProjectVisitorTest : public QObject
{
    Q_OBJECT
public:
    CMakeProjectVisitorTest();
    virtual ~CMakeProjectVisitorTest() {}

private slots:
    void testVariables();
    void testVariables_data();

    void init();

    void testRun();
    void testRun_data();

    void addTest_single();
    void addTest_multiple();
    void addTest_flags();
    void addTest_url();
    void addTest_identicalAddExecutable();
    void addTest_properties();
    void addTest_kde4AddUnitTest();
    void addTest_variableUsage();

private:
    // helpers
    CMakeProjectVisitor* setupVisitor();
    CMakeFileContent setupFileContent(const QString& content);
    void assertTestFound(CMakeProjectVisitor* visitor, const QString& testName, const QString& exe, const QStringList& args = QStringList());

private:
    KDevelop::TopDUContext* m_fakeContext;
    QString m_someCMakeLists;
};

#endif
