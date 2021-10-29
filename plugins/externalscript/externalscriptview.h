/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTVIEW_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTVIEW_H

#include <QWidget>

#include "ui_externalscriptview.h"

class ExternalScriptItem;
class QAction;

class QSortFilterProxyModel;

class ExternalScriptPlugin;

class ExternalScriptView
    : public QWidget
    , Ui::ExternalScriptViewBase
{
    Q_OBJECT

public:
    explicit ExternalScriptView(ExternalScriptPlugin* plugin, QWidget* parent = nullptr);
    ~ExternalScriptView() override;

    /// @return Currently selected script item.
    ExternalScriptItem* currentItem() const;
    /// @return Item for @p index.
    ExternalScriptItem* itemForIndex(const QModelIndex& index) const;

private Q_SLOTS:
    void contextMenu (const QPoint& pos);

    void addScript();
    void removeScript();
    void editScript();

    /// disables or enables available actions based on the currently selected item
    void validateActions();

protected:
    /// insert snippet on double click
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    ExternalScriptPlugin* m_plugin;
    QSortFilterProxyModel* m_model;
    QAction* m_addScriptAction;
    QAction* m_editScriptAction;
    QAction* m_removeScriptAction;
};

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTVIEW_H
