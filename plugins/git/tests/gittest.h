/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_GIT_INIT_H
#define KDEVPLATFORM_PLUGIN_GIT_INIT_H

#include <QtCore/QObject>
#include <QtCore/QString>

class GitPlugin;

namespace KDevelop
{
    class TestCore;
}

class GitInitTest: public QObject
{
    Q_OBJECT

private:
    void repoInit();
    void addFiles();
    void commitFiles();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testInit();
    void testAdd();
    void testCommit();
    void testBranching();
    void testBranch(const QString &branchName);
    void revHistory();
    void testAnnotation();
    void testRemoveEmptyFolder();
    void testRemoveEmptyFolderInFolder();
    void testRemoveUnindexedFile();
    void testRemoveFolderContainingUnversionedFiles();

private:
    GitPlugin* m_plugin;
    void removeTempDirs();
};

#endif
