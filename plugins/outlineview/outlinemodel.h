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

#include <serialization/indexedstring.h>

#include <QAbstractItemModel>
#include <vector>
#include <memory>

class OutlineNode;

namespace KDevelop {
class IDocument;
class DUContext;
class TopDUContext;
class Declaration;
class ParseJob;
}

class OutlineModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(OutlineModel)
public:
    explicit OutlineModel(QObject* parent = 0);
    ~OutlineModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void activate(const QModelIndex& realIndex);
private slots:
    void onParseJobFinished(KDevelop::ParseJob* job);
    void rebuildOutline(KDevelop::IDocument* doc);
private:
    std::unique_ptr<OutlineNode> m_rootNode;
    KDevelop::IDocument* m_lastDoc;
    KDevelop::IndexedString m_lastUrl;
};
