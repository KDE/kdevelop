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

#include "coverageexport.h"

namespace Veritas
{

/*! Coverage report table */
class VERITAS_COVERAGE_EXPORT DrillDownView: public QTableView
{
Q_OBJECT

public:
    DrillDownView(QWidget *parent = 0);
    virtual ~DrillDownView();
    void resizeFileStateColumns();

    /*!
     * Shows the coverage data for directories.
     * The selection is cleared, and then set again to the directories selected
     * before sliding to the left (including the current index).
     */
    void slideLeft();

    /*!
     * Shows the coverage data for the files in the currently selected
     * directory.
     * The selection is cleared.
     */
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
    int maxWidth() const;

private:
    QTimeLine animation;
    QPixmap oldView;
    QPixmap newView;
    int lastPosition;

    /*!
     * Stores the selection of directories to restore it when sliding left
     * again.
     */
    QItemSelection leftSelection;
};

}

#endif // VERITAS_COVERAGE_DRILLDOWNVIEW_H
