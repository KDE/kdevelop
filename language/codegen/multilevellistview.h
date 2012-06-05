/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_MULTILEVELLISTVIEW_H
#define KDEVELOP_MULTILEVELLISTVIEW_H

#include <QtGui/QWidget>
#include "../languageexport.h"

class QModelIndex;
class QListView;
class QAbstractItemModel;
namespace KDevelop
{

class KDEVPLATFORMLANGUAGE_EXPORT MultiLevelListView : public QWidget
{
    Q_OBJECT
public:
    explicit MultiLevelListView (int levels = 2, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~MultiLevelListView();
    
    void setModel(QAbstractItemModel* model);
    QListView* viewForLevel(int level);
    
    QModelIndex currentIndex();
    
    void addWidget(int level, QWidget* widget);
    
signals:
    void currentIndexChanged(const QModelIndex& current, const QModelIndex& previous);
    
public slots:
    void setRootIndex(const QModelIndex& index);
    void setCurrentIndex(const QModelIndex& index);
    
private slots:
    void currentChanged(int i);
    
private:
    class MultiLevelListViewPrivate* const d;
};

}

#endif // KDEVELOP_MULTILEVELLISTVIEW_H
