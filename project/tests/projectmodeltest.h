/***************************************************************************
 *   Copyright 2010 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVELOP_PROJECT_PROJECTMODELTEST_INCLUDED
#define KDEVELOP_PROJECT_PROJECTMODELTEST_INCLUDED

#include <QtCore/QObject>
#include <QtCore/QModelIndex>

class ProjectProxyModel;
namespace KDevelop
{
class ProjectModel;
}

class ModelTest;

class ProjectModelTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void testCreateFileSystemItems();
    void testCreateFileSystemItems_data();
    void testCreateTargetItems();
    void testCreateTargetItems_data();
    void testCreateSimpleHierarchy();
    void testItemSanity();
    void testRename();
    void testRename_data();
    void testChangeWithProxyModel();
    void testWithProject();
    void testTakeRow();
    void testItemsForUrl();
    void testItemsForUrl_data();
    void testProjectProxyModel();
    void testProjectFileSet();
    void testProjectFileIcon();
private:
    KDevelop::ProjectModel* model;
    ProjectProxyModel* proxy;
};

Q_DECLARE_METATYPE( QModelIndex )

#endif
