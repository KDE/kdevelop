/*
    SPDX-FileCopyrightText: 2021 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_WORKINGSETS_H
#define KDEVPLATFORM_TEST_WORKINGSETS_H

#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>

class QMainWindow;
class QWidget;

namespace KDevelop
{
class WorkingSetController;
class IDocumentController;
}

namespace Sublime
{
class Area;
}

class TestWorkingSetController : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanupTestCase();

    void createWorkingSet();
    void deleteWorkingSet();
    void switchArea();
    void restoreSplits();

private:
    void restartSession();

    QWidget* m_closedSets;
    KDevelop::WorkingSetController* m_workingSetCtrl;
    Sublime::Area* m_area;
    Sublime::Area* m_area_debug;
    KDevelop::IDocumentController* m_documentCtrl;

    QMainWindow* m_mainWindow;

    QTemporaryDir m_tempDir;
    QTemporaryFile m_file;
};

#endif // KDEVPLATFORM_TEST_WORKINGSETS_H
