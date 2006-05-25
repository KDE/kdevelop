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
#ifndef DDOCKWINDOW_H
#define DDOCKWINDOW_H

#include <qdockwindow.h>
#include <qvaluelist.h>

class QBoxLayout;
class QToolButton;
class QWidgetStack;
class KComboBox;

class DMainWindow;

namespace Ideal {
    class Button;
    class ButtonBar;
}

class DDockWindow : public QDockWindow {
    Q_OBJECT
public:
    enum Position { Bottom, Left, Right };

    DDockWindow(DMainWindow *parent, Position position);
    virtual ~DDockWindow();

    virtual void setVisible(bool v);
    bool visible() const { return m_visible; }
    Position position() const { return m_position; }

    virtual void addWidget(const QString &title, QWidget *widget);
    virtual void raiseWidget(QWidget *widget);
    /**Removes the widget from dock. Does not delete it.*/
    virtual void removeWidget(QWidget *widget);

    virtual void hideWidget(QWidget *widget);
    virtual void showWidget(QWidget *widget);

    virtual QWidget *currentWidget() const;

    virtual void setMovingEnabled(bool b);

    virtual void saveSettings();

    DMainWindow *mainWindow() const { return m_mainWindow; }

    virtual void selectLastWidget();

private slots:
    void selectWidget();
    void selectWidget(Ideal::Button *button);

protected:
    virtual void loadSettings();

    Ideal::ButtonBar *m_bar;
    QWidgetStack *m_widgetStack;

    QMap<Ideal::Button*, QWidget*> m_widgets;
    QMap<QWidget*, Ideal::Button*> m_buttons;

private:
    Position m_position;
    bool m_visible;
    QString m_name;
    DMainWindow *m_mainWindow;

    Ideal::Button *m_toggledButton;
    QBoxLayout *m_internalLayout;
};

#endif
