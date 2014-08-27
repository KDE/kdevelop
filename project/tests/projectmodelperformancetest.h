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

#ifndef KDEVELOP_PROJECT_PROJECTMODELPERFORMANCETEST_INCLUDED
#define KDEVELOP_PROJECT_PROJECTMODELPERFORMANCETEST_INCLUDED

#include <QWidget>
#include <QtCore/QStack>

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
    ProjectModelPerformanceTest(QWidget* parent = 0);
    virtual ~ProjectModelPerformanceTest();
private slots:
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
