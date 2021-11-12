/*
    This file was inspired by KDevelop's git plugin
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    Adapted for Perforce
    SPDX-FileCopyrightText: 2011 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PERFORCEPLUGIN_TEST_H
#define PERFORCEPLUGIN_TEST_H

#include <QObject>
#include <QString>

class PerforcePlugin;

class PerforcePluginTest : public QObject
{
    Q_OBJECT
public:
    PerforcePluginTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testAdd();
    void testEdit();
	void testEditMultipleFiles();
    void testStatus();
    void testAnnotate();
    void testHistory();
    void testRevert();
    void testUpdateFile();
    void testUpdateDir();
    void testCommit();
    void testDiff();
private:
    void removeTempDirsIfAny();
    void createNewTempDirs();

    PerforcePlugin* m_plugin = nullptr;

    const QString tempDir;
    const QString perforceTestBaseDirNoSlash;
    const QString perforceTestBaseDir;
    const QString perforceConfigFileName;

    const QString perforceSrcDir;
    const QString perforceTest_FileName;
    const QString perforceTest_FileName2;
    const QString perforceTest_FileName3;
};

#endif
