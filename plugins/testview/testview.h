/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <QtGui/QTreeView>

class TestViewPlugin;
class QStandardItemModel;

class TestView : public QTreeView
{
    Q_OBJECT
public:
    explicit TestView(TestViewPlugin* plugin, QWidget* parent = 0);
    virtual ~TestView();

public slots:
    void reloadTests();
    void buildTestModel();

private:
    TestViewPlugin* m_plugin;
    QStandardItemModel* m_model;
};

#endif // TESTVIEW_H
