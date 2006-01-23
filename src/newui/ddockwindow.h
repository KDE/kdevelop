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

#include "q3mainwindow.h"
#include <QMap>
#include <QList>
#include <QStackedWidget>

class QBoxLayout;
class QToolButton;
class QStackedWidget;
class KComboBox;

namespace Ideal {
    class Button;
    class ButtonBar;
}

class DDockWindow : public Q3DockWindow {
    Q_OBJECT
public:
    enum Position { Bottom, Left, Right };

    DDockWindow(QWidget *parent, Position position);
    virtual ~DDockWindow();

    inline void expand() { setExpanded(true); }
    inline void collapse() { setExpanded(false); }
    bool isExpanded() const { return m_expanded; }
    Position position() const { return m_position; }

    void addWidget(const QString &title, QWidget *widget);
    void raiseWidget(QWidget *widget);
    /**Removes the widget from dock. Does not delete it.*/
    void removeWidget(QWidget *widget);

    void hideWidget(QWidget *widget);
    void showWidget(QWidget *widget);

    QWidget *currentWidget() const;

    void setMovingEnabled(bool b);

private slots:
    void selectWidget();
    void selectWidget(Ideal::Button *button);

protected:
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void setExpanded(bool v);

    Ideal::ButtonBar *m_bar;
    QStackedWidget *m_widgetStack;

    QMap<Ideal::Button*, QWidget*> m_widgets;
    QMap<QWidget*, Ideal::Button*> m_buttons;

private:
    Position m_position;
    bool m_expanded;
    QString m_name;

    Ideal::Button *m_toggledButton;
    QBoxLayout *m_internalLayout;
};

#endif
