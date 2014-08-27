/***************************************************************************
 *   class CommitViewDelegate was partly taken from QGit                     *
 *              (Author: Marco Costalba (C) 2005-2007)                     *
 *                                                                         *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "commitView.h"

#include <QPainter>
#include <KDE/KDebug>

#include "../../../vcsrevision.h"
#include "commitlogmodel.h"

    enum LaneType {
//         EMPTY,
//         ACTIVE,
//         NOT_ACTIVE,
//         MERGE_FORK,
//         MERGE_FORK_R,
//         MERGE_FORK_L,
//         JOIN,
//         JOIN_R,
//         JOIN_L,
//         HEAD,
//         HEAD_R,
//         HEAD_L,
//         TAIL,
//         TAIL_R,
//         TAIL_L,
//         CROSS,
//         CROSS_EMPTY,
//         INITIAL,
        BRANCH/*,*/
//         UNAPPLIED,
//         APPLIED,
//         BOUNDARY,
//         BOUNDARY_C, // corresponds to MERGE_FORK
//         BOUNDARY_R, // corresponds to MERGE_FORK_R
//         BOUNDARY_L, // corresponds to MERGE_FORK_L
// 
//         LANE_TYPES_NUM
    };

CommitView::CommitView(QWidget *parent)
    : QTreeView(parent)
{
//     setLineHeight(fontMetrics().height());
    CommitViewDelegate* delegate = new CommitViewDelegate(this, this);
    setItemDelegate(delegate);
//     setupGeometry();
}

/*===============================================================================================*/

CommitViewDelegate::CommitViewDelegate(CommitView* _view, QObject* p) 
    : QItemDelegate(p) 
{
    view = _view;
}

void CommitViewDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt,
                               const QModelIndex& index) const 
{
    p->setRenderHints(QPainter::Antialiasing);

    if (index.column() == GRAPH_COLUMN)
        return paintGraph(p, opt, index);

    return QItemDelegate::paint(p, opt, index);
}

void CommitViewDelegate::paintGraph(QPainter* p, const QStyleOptionViewItem& opt,
                                    const QModelIndex& index) const {

    static const QColor colors[/*COLORS_NUM*/8] = { Qt::black, Qt::red, Qt::darkGreen /*DARK_GREEN*/,
                                                    Qt::blue, Qt::darkGray, /*BROWN*/Qt::darkYellow,
                                                    Qt::magenta, Qt::yellow /*ORANGE*/ };
    if (opt.state & QStyle::State_Selected)
        p->fillRect(opt.rect, opt.palette.highlight());
    else
        p->fillRect(opt.rect, opt.palette.base());

    QAbstractItemModel* model = view->model();

/*    const Rev* r = revLookup(i.row(), &fh);*/
//     if (!r)
//         return;

    p->save();
    p->translate(QPoint(opt.rect.left(), opt.rect.top()));

//     // calculate lanes
//     if (r->lanes.count() == 0)
//         git->setLane(r->sha(), fh);

    QBrush back = opt.palette.base();
//     const QVector<int>& lanes(r->lanes);
//     uint laneNum = model->rowCount();
    uint branchNum = (dynamic_cast<CommitLogModel*>(model))->branchCount(index.row());

    int x1 = 0; int x2 = 0;

//     int maxWidth = opt.rect.right();
    int lw = 12;  //TODO: count it with laneWidth(), it depends on font
        for(uint curBr = 0; curBr < branchNum; curBr++)
        {
            x1 = x2;
            x2 += lw;
            paintGraphLane(p, (dynamic_cast<CommitLogModel*>(model))->getProperties(index.row())[curBr], 
                           x1, x2, view->getLineHeight(index), colors[curBr % 8], back);
        }
    p->restore();
}


void CommitViewDelegate::paintGraphLane(QPainter* p, int type, int x1, int x2, const int height,
                                        const QColor& col, const QBrush& back) const 
{
    Q_UNUSED(back);

    int h = height / 2; //TODO: laneHeight!
    int m = (x1 + x2) / 2;
    int r = (x2 - x1) / 3;
    int d =  2 * r;

    #define P_CENTER m , h
    #define P_0      x2, h
    #define P_90     m , 0
    #define P_180    x1, h
    #define P_270    m , 2 * h
    #define R_CENTER m - r, h - r, d, d

    static QPen myPen(Qt::black, 2); // fast path here
    myPen.setColor(col);
    p->setPen(myPen);

    switch(type)
    {
    case DVcsEvent::INITIAL:
        p->drawLine(P_90, P_CENTER);
        break;
    case DVcsEvent::HEAD:
        p->drawLine(P_270, P_CENTER);
        break;
    case DVcsEvent::BRANCH:
    case DVcsEvent::MERGE:
    case DVcsEvent::CROSS:
        p->drawLine(P_90, P_270);
        break;
    case DVcsEvent::HCROSS:
        p->drawLine(P_0, P_180);
        break;
    case DVcsEvent::MERGE_RIGHT:
    {
        p->drawLine(P_180, P_CENTER);
        p->drawLine(P_CENTER, P_90);
        break;
    }
    }


//     // horizontal line
//     switch (type) {
//     case MERGE_FORK:
//     case JOIN:
//     case HEAD:
//     case TAIL:
//     case CROSS:
//     case CROSS_EMPTY:
//     case BOUNDARY_C:
//         p->drawLine(P_180, P_0);
//         break;
//     case MERGE_FORK_R:
//     case JOIN_R:
//     case HEAD_R:
//     case TAIL_R:
//     case BOUNDARY_R:
//         p->drawLine(P_180, P_CENTER);
//         break;
//     case MERGE_FORK_L:
//     case JOIN_L:
//     case HEAD_L:
//     case TAIL_L:
//     case BOUNDARY_L:
//         p->drawLine(P_CENTER, P_0);
//         break;
//     default:
//         break;
//     }

    // center symbol, e.g. rect or ellipse
    switch (type) {
    case DVcsEvent::INITIAL:
    case DVcsEvent::BRANCH:
    case DVcsEvent::HEAD:
    {
        p->setPen(Qt::NoPen);
        p->setBrush(col);
        p->drawEllipse(R_CENTER);
        break;
    }
    //we always have parent branch at the right
    case DVcsEvent::MERGE:
    {
        kDebug() << "MERGE!";
        p->setPen(Qt::NoPen);
        p->setBrush(col);
        p->drawLine(P_CENTER, x1, 0);
        p->drawRect(R_CENTER);
        break;
    }
    }

    #undef P_CENTER
    #undef P_0
    #undef P_90
    #undef P_180
    #undef P_270
    #undef R_CENTER
}
