/***************************************************************************
 *   Copyright 2005 Benjamin Meyer <ben@meyerhome.net>                     *
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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

#ifndef VERITAS_COVERAGE_DRILLDOWNVIEW_H
#define VERITAS_COVERAGE_DRILLDOWNVIEW_H

#include <QtGui/QtGui>

namespace Veritas
{

class DrillDownView: public QTableView
{
Q_OBJECT

public:
    DrillDownView(QWidget *parent = 0);
    virtual ~DrillDownView();
    void resizeFileStateColumns();
    void slideLeft();
    void slideRight(const QModelIndex&);

Q_SIGNALS:
    void returnPressed(const QModelIndex &index);
    void rootIndexChanged(const QModelIndex &index);
    void completedSlideLeft();
    void completedSlideRight();

public Q_SLOTS:
    void resizeDirStateColumns();
    void setRootIndex(const QModelIndex &index);

protected:
    virtual void paintEvent(QPaintEvent * event);
    virtual void keyPressEvent(QKeyEvent *event);

private Q_SLOTS:
    void slide(int x);

private:
    bool isBusy();
    void animateSlide(int moveDirection);

private:
    QTimeLine animation;
    QPixmap oldView;
    QPixmap newView;
    int lastPosition;
};

}

#endif // VERITAS_COVERAGE_DRILLDOWNVIEW_H
