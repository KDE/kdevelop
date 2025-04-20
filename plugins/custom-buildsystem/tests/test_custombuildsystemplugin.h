/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef TEST_CUSTOMBUILDSYSTEMPLUGIN_H
#define TEST_CUSTOMBUILDSYSTEMPLUGIN_H

#include <QObject>

namespace KDevelop
{
class IProject;
}

class TestCustomBuildSystemPlugin : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void loadSimpleProject();
    void buildDirProject();
    void loadMultiPathProject();
private:
    KDevelop::IProject* m_currentProject = nullptr;
};

#endif
