/*
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TEST_REFACTORING_H
#define TEST_REFACTORING_H

#include <QObject>

namespace KDevelop {
class TestProjectController;
}

class TestRefactoring : public QObject
{
    Q_OBJECT
public:
    ~TestRefactoring() override;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testClassRename();

private:
    KDevelop::TestProjectController* m_projectController;
};

#endif // TEST_REFACTORING_H
