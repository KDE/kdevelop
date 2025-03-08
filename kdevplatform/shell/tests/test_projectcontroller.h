/*
    SPDX-FileCopyrightText: 2008 Manuel Breugelmans <mbr.nxi@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVELOP_SHELL_TEST_PROJECTCONTROLLER_H
#define KDEVELOP_SHELL_TEST_PROJECTCONTROLLER_H

#include <QObject>
#include <QDir>

#include "util/path.h"

#include <memory>

class QSignalSpy;
namespace KDevelop
{
class Core;
class IProject;
class Project;
class ProjectController;
}

class FakeFileManager;

class TestProjectController : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void openProject();
    void closeProject();
    void openCloseOpen();
    void openMultiple();
    void reopen();
    void reopenWhileLoading();

    void emptyProject();
    void singleFile();
    void singleDirectory();
    void fileInSubdirectory();
    void prettyFileName_data();
    void prettyFileName();

    void changesModel();

private:
    KDevelop::Path writeProjectConfig(const QString& name);

    std::unique_ptr<QSignalSpy> createOpenedSpy();
    std::unique_ptr<QSignalSpy> createClosedSpy();
    std::unique_ptr<QSignalSpy> createClosingSpy();

    KDevelop::Project* assertProjectOpened(const QString& name);
    void assertSpyCaughtProject(QSignalSpy* spy, KDevelop::IProject* proj);
    void assertProjectClosed(KDevelop::IProject* proj);
    void assertEmptyProjectModel();

    FakeFileManager* createFileManager();

private:
    KDevelop::Path m_projFilePath;
    KDevelop::Core* m_core;
    KDevelop::ProjectController* m_projCtrl;
    QString m_projName;
    QList<KDevelop::Path> m_tmpConfigs;
    QDir m_scratchDir;
    KDevelop::Path m_projFolder;

    QList<FakeFileManager*> m_fileManagerGarbage;
};

#endif // KDEVELOP_SHELL_TEST_PROJECTCONTROLLER_H
