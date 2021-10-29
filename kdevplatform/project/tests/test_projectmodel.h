/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVELOP_PROJECT_TEST_PROJECTMODEL
#define KDEVELOP_PROJECT_TEST_PROJECTMODEL

#include <QObject>
#include <QModelIndex>

class ProjectProxyModel;
namespace KDevelop
{
class ProjectModel;
}

class TestProjectModel : public QObject
{
Q_OBJECT
private Q_SLOTS:
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
    void testItemsForPath();
    void testItemsForPath_data();
    void testProjectProxyModel();
    void testProjectFileSet();
    void testProjectFileIcon();
private:
    KDevelop::ProjectModel* model;
    ProjectProxyModel* proxy;
};

Q_DECLARE_METATYPE( QModelIndex )

#endif
