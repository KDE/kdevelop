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

#include <ktabwidget.h>
#include <QtCore/QMap>
#include <QtGui/QListView>
class QStandardItemModel;
class QString;
class SimpleOutputView;
class OutputViewCommand;
class QModelIndex;

class OutputWidget : public KTabWidget
{
    Q_OBJECT
    public:
        OutputWidget(QWidget* parent, SimpleOutputView* view);
    public Q_SLOTS:
        void addNewTab(const QString& title, QStandardItemModel* );

        // Note that *cmd should not be const. 
        void addNewTab( OutputViewCommand* cmd );

        void searchNextError();
        void searchPrevError();

    private:
        QMap<QString, QListView*> m_listviews;
};

/** @class OutputListView
 * Actual listview that will be embedded in OutputWidget's tab
 * Subclassed to handle context-menu and item activation.
 */
class OutputListView : public QListView
{
    Q_OBJECT
public:
    OutputListView( QWidget* parent );
    virtual ~OutputListView();

    virtual void setModel( QAbstractItemModel *model );

public Q_SLOTS:
    void highlightNextErrorItem();
    void highlightPrevErrorItem();

protected Q_SLOTS:
    void slotActivated( const QModelIndex& index );
    void customContextMenuRequested( const QPoint & point );

private:
    class OutputListViewPrivate * d;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
