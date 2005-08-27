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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DMAINWINDOW_H
#define DMAINWINDOW_H

#include <kxmlguiclient.h>
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
    DDockWindow *toolWindow(DDockWindow::Position position) const;
    
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
    
    DDockWindow *m_leftDock;
    DDockWindow *m_rightDock;
    DDockWindow *m_bottomDock;

    Ideal::DockSplitter *m_central;
    DTabWidget *m_activeTabWidget;
    
    QValueList<DTabWidget*> m_tabs;
    
    bool m_openTabAfterCurrent;
    bool m_showIconsOnTabs;
    bool m_firstRemoved;
    
    QValueList<QWidget*> m_widgets;
    QMap<QWidget*, DTabWidget*> m_widgetTabs;
    QWidget *m_currentWidget;

private slots:
    void invalidateActiveTabWidget();

};

#endif
