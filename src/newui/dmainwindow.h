/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef DMAINWINDOW_H
#define DMAINWINDOW_H

#include <kparts/mainwindow.h>

#include "ddockwindow.h"

class DTabWidget;
namespace Ideal {
    class DockSplitter;
}

/**Main window which provides simplified IDEA mode.*/
class DMainWindow: public KParts::MainWindow {
    Q_OBJECT
public:
    DMainWindow(QWidget *parent = 0, const char *name = 0);
    virtual ~DMainWindow();

    /**@return The tool window in given @p position.*/
    DDockWidget *toolWindow(Qt::DockWidgetArea area) const;

    /**Adds a tabbed widget into the active (focused) tab widget.
    If @p widget is null then only tab is created.*/
    virtual void addWidget(QWidget *widget, const QString &title);
    virtual void addWidget(DTabWidget *tab, QWidget *widget, const QString &title);
    /**Removes widget. Does not delete it.*/
    virtual void removeWidget(QWidget *widget);

public slots:
    DTabWidget *splitHorizontal();
    DTabWidget *splitVertical();

protected slots:
    /**This does nothing. Reimplement in subclass to close the tab
    when corner close button is pressed.*/
    virtual void closeTab();
    /**This does nothing. Reimplement in subclass to close the tab
    when hover close button is pressed.*/
    virtual void closeTab(QWidget*);
    /**This does nothing. Reimplement in subclass to show tab context menu.*/
    virtual void tabContext(QWidget*,const QPoint &);

signals:
    void widgetChanged(QWidget *);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

    virtual void loadSettings();

    virtual void createToolWindows();
    virtual DTabWidget *createTab();

    DDockWidget *m_leftDock;
    DDockWidget *m_rightDock;
    DDockWidget *m_bottomDock;

    Ideal::DockSplitter *m_central;
    DTabWidget *m_activeTabWidget;

    QList<DTabWidget*> m_tabs;

    bool m_openTabAfterCurrent;
    bool m_showIconsOnTabs;
    bool m_firstRemoved;

    QList<QWidget*> m_widgets;
    QMap<QWidget*, DTabWidget*> m_widgetTabs;
    QWidget *m_currentWidget;

private slots:
    void invalidateActiveTabWidget();

};

#endif
