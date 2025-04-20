/*
    This file was partly taken from KDevelop's cvs plugin
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>

    Adapted for Git
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_GIT_INIT_H
#define KDEVPLATFORM_PLUGIN_GIT_INIT_H

#include <QObject>

class GitPlugin;

class GitInitTest: public QObject
{
    Q_OBJECT

private:
    void repoInit();
    void addFiles();
    void commitFiles();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testInit();
    void testReadAndSetConfigOption();
    void testAdd();
    void testCommit();
    void testBranching();
    void testBranch(const QString &branchName);
    void testMerge();
    void revHistory();
    void testAnnotation();
    void testRemoveEmptyFolder();
    void testRemoveEmptyFolderInFolder();
    void testRemoveUnindexedFile();
    void testRemoveFolderContainingUnversionedFiles();
    void testDiff();
    void testStash();
    void testRegisterRepositoryForCurrentBranchChanges();

private:
    GitPlugin* m_plugin;
    void removeTempDirs();
};

#endif
