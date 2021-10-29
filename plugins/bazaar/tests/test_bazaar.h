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

class BazaarPlugin;

namespace KDevelop
{
    class TestCore;
}

class TestBazaar: public QObject
{
    Q_OBJECT

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
    BazaarPlugin* m_plugin;
    void removeTempDirs();
};

#endif // KDEVPLATFORM_PLUGIN_TEST_BAZAAR_H
