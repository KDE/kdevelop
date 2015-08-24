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

#include <QMainWindow>

class ProblemTreeView;

namespace KDevelop
{
struct ModelData;

/**
 * @brief Provides a tabbed view for models in the ProblemModelSet.
 *
 * Also provides a toolbar for actions for the models and shows the number of messages in each tab's text.
 * When the load() method is called it looks up the models in the ProblemModelSet.
 * For each model it creates a treeview, which is then added to the tabbed view and a new tab.
 * The tab's text will be the name of the model + the number of items in the treeview.
 *
 * TODO: According to apol this should NOT be a QMainWindow,
 * because updating the widget's actions should be sufficient to update the
 * toolbar of the toolviiew
 */
class ProblemsView : public QMainWindow, public IToolViewActionListener
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IToolViewActionListener)

public:
    explicit ProblemsView(QWidget* parent = NULL);
    ~ProblemsView();

    /// Load all the current models and create tabs for them
    void load();

public Q_SLOTS:
    /// Triggered when a new model is added to the ModelSet
    void onModelAdded(const ModelData& data);

    /// Triggered when a model is removed from the ModelSet
    void onModelRemoved(const QString& name);

    /// Triggered when the user (or program) selects a new tab
    void onCurrentChanged(int idx);

    /// Triggered when a view changes (happens when the model data changes)
    void onViewChanged();

    void selectNextItem() override;
    void selectPreviousItem() override;

private:
    ProblemTreeView* currentView() const;

    /// Create a view for the model and add to the tabbed widget
    void addModel(const ModelData& data);

    /// Update the toolbar with the widget's actions
    void updateToolBar();

    /// Update the tab's text (name + number of problems in that tab)
    void updateTab(int idx, int rows);

    QToolBar* m_toolBar;
    QTabWidget* m_tabWidget;
};
}

#endif
