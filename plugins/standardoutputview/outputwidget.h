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

#include <QWidget>
#include <QtCore/QMap>
#include <outputview/ioutputviewmodel.h>
#include <outputview/ioutputview.h>

class QAbstractProxyModel;
class QAbstractItemView;
class QString;
class StandardOutputView;
class QSignalMapper;
class QStackedWidget;
class QTreeView;
class QToolButton;
class QWidgetAction;
class QSortFilterProxyModel;
class QModelIndex;
class ToolViewData;
class KTabWidget;
class KToggleAction;
class QAction;
class KAction;
class KLineEdit;
class StandardOutputViewTest;

class OutputWidget : public QWidget
{
Q_OBJECT

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
    void selectNextItem();
    void selectPrevItem();
    void activate(const QModelIndex&);
    void scrollToIndex( const QModelIndex& );
    void setTitle(int outputId, const QString& title);

Q_SIGNALS:
    void outputRemoved( int, int );

private slots:
    void nextOutput();
    void previousOutput();
    void rowsInserted(const QModelIndex&, int, int);
    void copySelection();
    void selectAll();
    void outputFilter(const QString filter);
    void updateFilter(int index);
    void delayedScroll();

private:
    QTreeView* createListView(int id);
    void setCurrentWidget( QTreeView* view );
    QWidget* currentWidget() const;
    void enableActions();
    KDevelop::IOutputViewModel* outputViewModel() const;
    QAbstractItemView* outputView() const;
    void activateIndex(const QModelIndex& index, QAbstractItemView* view, KDevelop::IOutputViewModel* iface);
    void eventuallyDoFocus();
    int currentOutputIndex();
    void delayedScroll(QTreeView* view);

    QMap<int, QTreeView*> views;
    struct DelayData
    {
        QTimer* timer;
        int from;
        int to;
    };
    QMap<QTreeView*, DelayData> m_scrollDelay;
    QMap<int, QSortFilterProxyModel*> proxyModels;
    QMap<int, QString> filters;
    KTabWidget* tabwidget;
    QStackedWidget* stackwidget;
    const ToolViewData* data;
    QToolButton* m_closeButton;
    QAction* m_closeOthersAction;
    QAction* nextAction;
    QAction* previousAction;
    KToggleAction* activateOnSelect;
    KToggleAction* focusOnSelect;
    KLineEdit *filterInput;
    QWidgetAction* filterAction;
};

#endif

