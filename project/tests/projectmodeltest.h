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

namespace KDevelop
{
class ProjectModel;
}

class ProjectModelTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void testCreateFileSystemItems();
    void testCreateFileSystemItems_data();
    void testCreateTargetItems();
    void testCreateTargetItems_data();
private:
    KDevelop::ProjectModel* model;
};

#endif
