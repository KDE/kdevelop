/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PROBLEMSVIEW_H
#define PROBLEMSVIEW_H

#include <interfaces/itoolviewactionlistener.h>

#include <QWidget>

class ProblemTreeView;

class KActionMenu;
class KExpandableLineEdit;

class QAction;
class QActionGroup;
class QLineEdit;
class QTabWidget;

namespace KDevelop
{

struct ModelData;

/**
 * @brief Provides a tabbed view for models in the ProblemModelSet.
 *
 *
 * Also provides a toolbar for actions for the models and shows the number of messages in each tab's text.
 * When the load() method is called it looks up the models in the ProblemModelSet.
 * For each model it creates a treeview, which is then added to the tabbed view and a new tab.
 * The tab's text will be the name of the model + the number of items in the treeview.
 */
class ProblemsView : public QWidget, public IToolViewActionListener
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IToolViewActionListener)

public:
    explicit ProblemsView(QWidget* parent = nullptr);
    ~ProblemsView() override;

    /// Load all the current models and create tabs for them
    void load();

public Q_SLOTS:
    /// Triggered when a new model is added to the ModelSet
    void onModelAdded(const ModelData& data);

    /// Triggered when a model is removed from the ModelSet
    void onModelRemoved(const QString& id);

    /// Triggered when the user (or program) selects a new tab
    void onCurrentChanged(int idx);

    /// Triggered when a view changes (happens when the model data changes)
    void onViewChanged();

    /// Open tab for selected model
    void showModel(const QString& id);

    void selectNextItem() override;
    void selectPreviousItem() override;

private:
    ProblemTreeView* currentView() const;

    void setupActions();
    void updateActions();

    void handleSeverityActionToggled();
    void setScope(int scope);

    /// Create a view for the model and add to the tabbed widget
    void addModel(const ModelData& data);

    /// Update the tab's text (name + number of problems in that tab)
    void updateTab(int idx, int rows);

    QTabWidget* m_tabWidget;

    KActionMenu* m_scopeMenu = nullptr;
    KActionMenu* m_groupingMenu = nullptr;
    QAction* m_fullUpdateAction = nullptr;
    QAction* m_showImportsAction = nullptr;
    QActionGroup* m_severityActions = nullptr;
    QAction* m_currentDocumentAction = nullptr;
    QAction* m_showAllAction = nullptr;
    QAction* m_errorSeverityAction = nullptr;
    QAction* m_warningSeverityAction = nullptr;
    QAction* m_hintSeverityAction = nullptr;

    void setFilter(const QString& filterText);
    void setFilter(const QString& filterText, int tabIdx);

    KExpandableLineEdit* m_filterEdit;
    int m_prevTabIdx;
    QVector<ModelData> m_models;
};
}

#endif
