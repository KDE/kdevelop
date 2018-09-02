/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_OUTPUTWIDGET_H
#define KDEVPLATFORM_PLUGIN_OUTPUTWIDGET_H

#include <QMap>
#include <QWidget>
#include <QSharedPointer>

#include <interfaces/itoolviewactionlistener.h>
#include <outputview/ioutputviewmodel.h>
#include <outputview/ioutputview.h>

class KExpandableLineEdit;
class KToggleAction;
class StandardOutputViewTest;
class QAction;
class QAbstractItemView;
class QLineEdit;
class QModelIndex;
class QSortFilterProxyModel;
class QStackedWidget;
class QString;
class QTabWidget;
class QToolButton;
class QTreeView;
class QWidgetAction;
class ToolViewData;

class OutputWidget : public QWidget, public KDevelop::IToolViewActionListener
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IToolViewActionListener)

    friend class StandardOutputViewTest;

public:
    OutputWidget(QWidget* parent, const ToolViewData* data);
    void removeOutput( int id );
    void raiseOutput( int id );
public Q_SLOTS:
    void addOutput( int id );
    void changeModel( int id );
    void changeDelegate( int id );
    void closeActiveView();
    void closeOtherViews();
    void selectFirstItem();
    void selectNextItem() override;
    void selectPreviousItem() override;
    void selectLastItem();
    void activate(const QModelIndex&);
    void scrollToIndex( const QModelIndex& );
    void setTitle(int outputId, const QString& title);

Q_SIGNALS:
    void outputRemoved( int, int );

private Q_SLOTS:
    void nextOutput();
    void previousOutput();
    void copySelection();
    void selectAll();
    void outputFilter(const QString& filter);
    void updateFilter(int index);
    void clearModel();

private:
    enum SelectionMode {
        Last,
        Next,
        Previous,
        First
    };
    void selectItem(SelectionMode selectionMode);

    QTreeView* createListView(int id);
    void setCurrentWidget( QTreeView* view );
    QWidget* currentWidget() const;
    void enableActions();
    KDevelop::IOutputViewModel* outputViewModel() const;
    QAbstractItemView* outputView() const;
    void activateIndex(const QModelIndex& index, QAbstractItemView* view, KDevelop::IOutputViewModel* iface);
    void eventuallyDoFocus();
    int currentOutputIndex();

    struct FilteredView {
        QSharedPointer<QTreeView> view;
        QSharedPointer<QSortFilterProxyModel> proxyModel;
        QString filter;
    };
    QHash<int, FilteredView>::iterator findFilteredView(QAbstractItemView *view);

    QHash<int, FilteredView> m_views;
    QTabWidget* m_tabwidget;
    QStackedWidget* m_stackwidget;
    const ToolViewData* data;
    QToolButton* m_closeButton;
    QAction* m_closeOthersAction;
    QAction* m_nextAction;
    QAction* m_previousAction;
    KToggleAction* m_activateOnSelect;
    KToggleAction* m_focusOnSelect;
    KExpandableLineEdit* m_filterInput;
    QWidgetAction* m_filterAction;
};

#endif

