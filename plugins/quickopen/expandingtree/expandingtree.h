/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXPANDINGTREE_H
#define KDEVPLATFORM_PLUGIN_EXPANDINGTREE_H

#include <QTreeView>
#include <QTextDocument>

//A tree that allows drawing additional information
class ExpandingTree
    : public QTreeView
{
    Q_OBJECT
public:
    explicit ExpandingTree(QWidget* parent);

    enum CustomRoles {
        ProjectPathRole = Qt::UserRole + 5000
    };

    void setModel(QAbstractItemModel* model) override;
protected:
    void drawRow (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    int sizeHintForColumn (int column) const override;

    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
private:
    mutable QTextDocument m_drawText;
};

#endif // KDEVPLATFORM_PLUGIN_EXPANDINGTREE_H
