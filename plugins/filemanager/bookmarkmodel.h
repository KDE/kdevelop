/*
 *    This file is part of KDevelop, made within the KDE community.
 *
 *    SPDX-FileCopyrightText: 2025 Morten Danielsen Volden <mvolden2@gmail.com>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KDEV_PLUGIN_BOOKMARKMODEL_H
#define KDEV_PLUGIN_BOOKMARKMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include <QObject>

#include <KTextEditor/Document>

class KBookmark;

namespace KDevelop {
class IDocument;

class BookmarkModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ColumnIds {
        LocationColumnId,
        TitleColumnId,
        NumColumns
    };

public:
    explicit BookmarkModel(QObject* parent = nullptr);
    ~BookmarkModel() override;

public: // QAbstractTableModel API
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

public:
    const KBookmark& bookmark(const QModelIndex& index) const;
    QModelIndex index(const KBookmark& bookmark, int column = BookmarkModel::TitleColumnId) const;
    using QAbstractTableModel::index;

private Q_SLOTS:
    void markChanged(KTextEditor::Document* document, KTextEditor::Mark mark,
                     KTextEditor::Document::MarkChangeAction action);
    void onHasBookmarksChanged(bool hasBookmarks);
    void onBookmarksChanged();
    void onBookmarksChanged(const QList<int>& bookmarkIndizes);
    void onOffsetCodingChanged(int offsetCoding);
    void textDocumentCreated(KDevelop::IDocument*);

private:
    static const QPixmap bookmarkPixmap();
    QFont mFixedFont;
};

}

#endif
