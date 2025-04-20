/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef TEST_DEFINESANDINCLUDES_H
#define TEST_DEFINESANDINCLUDES_H

#include <QObject>

namespace KDevelop
{
class IProject;
}

class TestDefinesAndIncludes : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void loadSimpleProject();
    void loadMultiPathProject();
    void testNoProjectIncludeDirectories();
    void testEmptyProject();
private:
    KDevelop::IProject* m_currentProject = nullptr;
};

#endif
