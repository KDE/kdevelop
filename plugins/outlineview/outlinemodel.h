/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <QAbstractItemModel>

class OutlineNode;

namespace KDevelop {
class IDocument;
class DUContext;
class TopDUContext;
class Declaration;
}

class OutlineModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit OutlineModel(QObject* parent = 0);
    virtual ~OutlineModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void activate(QModelIndex realIndex);
private slots:
    void onDocumentSaved(KDevelop::IDocument* doc);
    void rebuildOutline(KDevelop::IDocument* doc);
private:
    QList<OutlineNode*> m_topLevelItems;
    KDevelop::IDocument* m_lastDoc;
    Q_DISABLE_COPY(OutlineModel)
};
