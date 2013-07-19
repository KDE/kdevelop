/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#ifndef CMAKELOADPROJECTTEST_H
#define CMAKELOADPROJECTTEST_H

#include <QtTest/QtTest>

class CMakeProjectData;
class CMakeProjectVisitor;
class CMakeAst;

/**
 * A test for the CMake parser and lexer loading three different real projects
 * Useful for doing call-analyzation and memory checking with valgrind
 * @author Matt Rogers <mattr@kde.org>
 */
class CMakeLoadProjectTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    //Note: These could be done as data-driven tests, but they are separate so that one can
    // more easily profile a plain-cmake vs. qt4-only vs. kde4 project
    void testTinyCMakeProject();
    void testSmallQt4Project();
    void testSmallKDE4Project();
    void testSmallProjectWithTests();
    void testKDE4ProjectWithTests();
    
private:
    CMakeProjectData parseProject( const QString& sourcedir );
};

#endif
