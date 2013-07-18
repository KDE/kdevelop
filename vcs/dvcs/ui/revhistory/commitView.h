/***************************************************************************
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

#ifndef KDEVPLATFORM_COMMIT_VIEW_H
#define KDEVPLATFORM_COMMIT_VIEW_H

#include <QtGui/QTreeView>
#include <QtGui/QItemDelegate>

enum ViewColumns {
    GRAPH_COLUMN = 0,
    SLOG_COLUMN = 1,
    AUTHOR_COLUMN = 2,
    DATE_COLUMN = 3
};

class CommitView : public QTreeView
{
    Q_OBJECT
public:
    CommitView(QWidget *parent = 0);
    ~CommitView() {};
//     void setLineHeight(int h) { lineHeight = h; }
    int getLineHeight(const QModelIndex & index) {return rowHeight(index);}
private:
//     int lineHeight;
};

class CommitViewDelegate : public QItemDelegate {
    Q_OBJECT
public:
    CommitViewDelegate(CommitView* _view, QObject* parent);

    void paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex &i) const;

private:
    void paintGraph(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex &i) const;
    void paintGraphLane(QPainter* p, int type, int x1, int x2, const int height, const QColor& col, const QBrush& back) const;

private:
    CommitView* view;

};

#endif
