/*
    SPDX-FileCopyrightText: 2011 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit TestQMakeProject(QObject* parent = nullptr);
    ~TestQMakeProject() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testBuildDirectory_data();
    void testBuildDirectory();
};

#endif
