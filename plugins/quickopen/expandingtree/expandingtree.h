/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_EXPANDINGTREE_H
#define KDEVPLATFORM_PLUGIN_EXPANDINGTREE_H

#include <QTreeView>
#include <QTextDocument>

//A tree that allows drawing additional information
class ExpandingTree : public QTreeView
{
   Q_OBJECT
 public:
    explicit ExpandingTree(QWidget* parent);

    enum CustomRoles {
        ProjectPathRole = Qt::UserRole + 5000
    };

    void setModel(QAbstractItemModel* model) override;

  protected:
    void drawRow ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    int sizeHintForColumn ( int column ) const override;

    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
  private:
    mutable QTextDocument m_drawText;
};

#endif // KDEVPLATFORM_PLUGIN_EXPANDINGTREE_H
