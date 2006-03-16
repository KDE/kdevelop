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
#ifndef DDOCKWINDOW_H
#define DDOCKWINDOW_H

#include <QMap>
#include <QList>
#include <QStackedWidget>
#include <QDockWidget>

class QStackedWidget;
class QToolBar;
class QActionGroup;
class KAction;
class KMainWindow;

class DDockWidget : public QDockWidget {
    Q_OBJECT
public:
    DDockWidget(Qt::DockWidgetArea area, KMainWindow* mainWindow);
    virtual ~DDockWidget();

    KMainWindow* mainWindow() const;

    inline void expand() { setExpanded(true); }
    inline void collapse() { setExpanded(false); }
    bool isExpanded() const { return m_expanded; }

    void addWidget(const QString &title, QWidget *widget);
    void raiseWidget(QWidget *widget);
    /**Removes the widget from dock. Does not delete it.*/
    void removeWidget(QWidget *widget);

    void hideWidget(QWidget *widget);
    void showWidget(QWidget *widget);

    QWidget *currentWidget() const;

private slots:
    void selectWidget(QAction* action);

protected:
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void setExpanded(bool v);
    virtual void resizeEvent(QResizeEvent* event);

private:
    void setSelectedAction(KAction* action);

    QStackedWidget* m_widgetStack;
    QToolBar* m_selectionBar;

    QActionGroup* m_selectActionGroup;
    QMap<KAction*, QWidget*> m_widgets;
    QMap<QWidget*, KAction*> m_actions;

    KAction* m_selectedAction;

    bool m_expanded;
    Qt::DockWidgetArea m_area;
};

#endif
