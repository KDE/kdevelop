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
class QAbstractItemModel;
class QString;
class StandardOutputView;
class OutputViewCommand;
class QModelIndex;
class QListView;
class QToolButton;
class QScrollBar;

class OutputWidget : public KTabWidget
{
    Q_OBJECT
    public:
        OutputWidget(QWidget* parent, StandardOutputView* view);
    public Q_SLOTS:
        void changeModel( int id );
        void changeDelegate( int id );
        void removeView( int id );
        void closeActiveView();
        void selectNextItem();
        void selectPrevItem();
        void activate(const QModelIndex&);
        void rangeChanged(int min, int max);
        void valueChanged(int value);

    Q_SIGNALS:
        void viewRemoved( int );
//         void activated( const QModelIndex& );

    private:
        QListView* createListView(int id);
        
        QMap<int, QListView*> m_listviews;

        // 0 == not at end, 1 == at end, 2 == no auto scroll
        QMap<QScrollBar*, int> m_sliders;

        QMap<QWidget*, int> m_widgetMap;
        StandardOutputView* m_outputView;
        QToolButton* m_closeButton;
        bool m_atEndOfUpdatingView;
};

#endif

