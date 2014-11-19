/* KDevelop QMake Support
 *
 * Copyright 2011 Julien Desgats <julien.desgats@gmail.com>
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

#ifndef TEST_QMAKEPROJECT_H
#define TEST_QMAKEPROJECT_H

#include <QObject>

/**
 * Test project handling related features. Very basic at moment, should be completed.
 * Project needed for tests are located in manual directory, and path is accessible by
 * QMAKE_TESTS_PROJECTS_DIR macro.
 */
class TestQMakeProject : public QObject {
    Q_OBJECT
public:
    TestQMakeProject(QObject* parent = 0);
    virtual ~TestQMakeProject();
private slots:
    void testBuildDirectory_data();
    void testBuildDirectory();
};

#endif
