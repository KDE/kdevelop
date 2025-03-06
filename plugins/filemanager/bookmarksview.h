/*
 *    This file is part of KDevelop, made within the KDE community.
 *
 *    SPDX-FileCopyrightText: 2025 Morten Danielsen Volden <mvolden2@gmail.com>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KDEV_PLUGIN_BOOKMARKSVIEW_H
#define KDEV_PLUGIN_BOOKMARKSVIEW_H

#include <QObject>
#include <QWidget>

class QTreeView;
class QLabel;
class QAction;
class QModelIndex;

namespace KDevelop {
class BookmarkModel;

class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksView(QWidget* parent = nullptr);
    ~BookmarksView() override;

public Q_SLOTS:
    QList<QAction*> contextMenuActions()
    {
        return m_contextMenuActions;
    };

private Q_SLOTS:
    void updateEmptyListOverlayLabel();

    void onBookmarkDoubleClicked(const QModelIndex& index);
    void onBookmarkSelectionChanged();
    void onCreateBookmarkButtonClicked();
    void onDeleteBookmarkButtonClicked();
    void onGotoBookmarkButtonClicked();
    void onRenameBookmarkButtonClicked();

private:
    BookmarkModel* mBookmarkListModel;

    QList<QAction*> m_contextMenuActions;

    QTreeView* mBookmarkListView;
    QLabel* m_emptyListOverlayLabel;
    QAction* mCreateBookmarkAction;
    QAction* mDeleteBookmarksAction;
    QAction* mGotoBookmarkAction;
    QAction* mRenameBookmarkAction;
};
}

#endif
