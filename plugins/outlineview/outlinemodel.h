/*
    SPDX-FileCopyrightText: 2010, 2015 Alex Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
}

class OutlineModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(OutlineModel)
public:
    explicit OutlineModel(QObject* parent = nullptr);
    ~OutlineModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public Q_SLOTS:
    void activate(const QModelIndex& realIndex);
private Q_SLOTS:
    void rebuildOutline(KDevelop::IDocument* doc);
private:
    std::unique_ptr<OutlineNode> m_rootNode;
    KDevelop::IDocument* m_lastDoc;
    KDevelop::IndexedString m_lastUrl;
};
