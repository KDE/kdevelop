/***************************************************************************
 *   Copyright (C) 2005 by Benjamin Meyer                                  *
 *   ben@meyerhome.net                                                     *
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
#ifndef DRILLDOWNVIEW_H
#define DRILLDOWNVIEW_H

#include <QtGui/QtGui>

class DrillDownView: public QListView {
     Q_OBJECT
public:
    DrillDownView(QWidget *parent = 0);
//     QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

public slots:
//     void currentChanged( const QModelIndex &current, const QModelIndex &previous );
    void slide(int x);

protected:
    virtual void paintEvent(QPaintEvent * event);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    QTimeLine animation;
    QPixmap oldView;
    QPixmap newView;
    int lastPosition;

};

#endif
