/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERTREEVIEW_H
#define KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERTREEVIEW_H

#include <QTreeView>

class DocumentSwitcherPlugin;

class DocumentSwitcherTreeView : public QTreeView
{
    Q_OBJECT

public:
    enum Roles {
        ProjectRole = Qt::UserRole + 1
    };
    explicit DocumentSwitcherTreeView(DocumentSwitcherPlugin* plugin);

    using QTreeView::sizeHintForColumn;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void drawBranches(QPainter* painter, const QRect& rect,
                      const QModelIndex& index) const override;

private:
    DocumentSwitcherPlugin* plugin;
};

#endif // KDEVPLATFORM_PLUGIN_DOCUMENTSWITCHERTREEVIEW_H
