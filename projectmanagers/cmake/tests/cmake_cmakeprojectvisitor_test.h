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
#include "cmakeprojectvisitor.h"


class CMakeProjectVisitorTest : public QObject, public CMakeProjectVisitor
{
    Q_OBJECT
public:
    CMakeProjectVisitorTest();

private slots:
    void init();
    void cleanup();

    void testVariables();
    void testVariables_data();

    void testRun();
    void testRun_data();
    
    void testFinder_init();
    void testFinder();
    void testFinder_data();

    void testGlobs();
    void testGlobs_data();

    void testForeachLines();

    void testTargetProperties();
    void testTargetProperties_data();
private:
    QStringList modulePath;    
    QStringList buildstrap;
    VariableMap initialVariables;
    KDevelop::ReferencedTopDUContext fakeContext;
};

#endif
