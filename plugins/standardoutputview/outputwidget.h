/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QMap>
class QString;
class StandardOutputView;
class QStackedWidget;
class QListView;
class QToolButton;
class ToolViewData;
class KTabWidget;
class QModelIndex;

class OutputWidget : public QWidget
{
Q_OBJECT
public:
    OutputWidget(QWidget* parent, ToolViewData* data);
    void removeOutput( int id );
    void raiseOutput( int id );
public Q_SLOTS:
    void addOutput( int id );
    void changeModel( int id );
    void changeDelegate( int id );
    void closeActiveView();
    void selectNextItem();
    void selectPrevItem();
    void activate(const QModelIndex&);

Q_SIGNALS:
    void outputRemoved( int, int );
private:
    QListView* createListView(int id);
    void setCurrentWidget( QListView* view );
    QWidget* currentWidget();

    QMap<int, QListView*> views;
    KTabWidget* tabwidget;
    QStackedWidget* stackwidget;
    ToolViewData* data;
    QToolButton* m_closeButton;
};

#endif

