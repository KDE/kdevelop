/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVELOP_PROJECT_PROJECTMODELPERFORMANCETEST_INCLUDED
#define KDEVELOP_PROJECT_PROJECTMODELPERFORMANCETEST_INCLUDED

#include <QWidget>
#include <QStack>

namespace KDevelop
{
class ProjectModel;
class ProjectBaseItem;
}
class QTreeView;

class ProjectModelPerformanceTest : public QWidget
{
Q_OBJECT
public:
    explicit ProjectModelPerformanceTest(QWidget* parent = nullptr);
    ~ProjectModelPerformanceTest() override;
private Q_SLOTS:
    void init();
    void addSmallTree();
    void addBigTree();
    void addBigTreeDelayed();
    void addItemDelayed();
private:
    QStack<KDevelop::ProjectBaseItem*> currentParent;
    int originalWidth;
    KDevelop::ProjectModel* model;
    QTreeView* view;
};

#endif
