/*
    This file was partly taken from KDevelop's cvs plugin
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>

    Adapted for Git
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    Adapted for Bazaar
    SPDX-FileCopyrightText: 2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_TEST_BAZAAR_H
#define KDEVPLATFORM_PLUGIN_TEST_BAZAAR_H

#include <QObject>
#include <QString>

class BazaarPlugin;

class TestBazaar: public QObject
{
    Q_OBJECT
public:
    TestBazaar();

private:
    void repoInit();
    void addFiles();
    void prepareWhoamiInformations();
    void commitFiles();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testInit();
    void testAdd();
    void testCommit();
    void testAnnotation();
    void testRemoveEmptyFolder();
    void testRemoveEmptyFolderInFolder();
    void testRemoveUnindexedFile();
    void testRemoveFolderContainingUnversionedFiles();

private:
    void removeTempDirs();

    BazaarPlugin* m_plugin = nullptr;

    const QString tempDir;
    const QString bazaarTest_BaseDir;
    const QString bazaarRepo;
    const QString bazaarSrcDir;
    const QString bazaarTest_FileName;
    const QString bazaarTest_FileName2;
    const QString bazaarTest_FileName3;
};

#endif // KDEVPLATFORM_PLUGIN_TEST_BAZAAR_H
