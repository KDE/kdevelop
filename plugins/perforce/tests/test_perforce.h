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

class PerforcePlugin;

class PerforcePluginTest : public QObject
{
    Q_OBJECT
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
    PerforcePlugin* m_plugin;
    void removeTempDirsIfAny();
    void createNewTempDirs();
};

#endif
