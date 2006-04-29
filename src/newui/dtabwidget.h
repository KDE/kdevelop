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
#ifndef DTABWIDGET_H
#define DTABWIDGET_H

#include <ktabwidget.h>
#include "dexport.h"

class QToolButton;

class DUI_EXPORT DTabWidget: public KTabWidget {
    Q_OBJECT
public:
    DTabWidget(QWidget *parent=0, const char *name=0);

    /**@return The close button at the top right corner.
    May be 0 if the configuration do not allow close buttons or the tabbar.*/
    QToolButton *closeButton() const;

    virtual void insertTab(QWidget *child, const QString &label, int index = -1 );
    virtual void insertTab(QWidget *child, const QIcon &iconset,
        const QString &label, int index = -1);

protected:
    virtual void loadSettings();
    virtual void saveSettings();

private slots:
    void setFocus(QWidget *w);
//    void updateHistory(QWidget *w);

private:
    bool m_tabBarShown;
    bool m_closeOnHover;
    bool m_closeButtonShown;

    QToolButton *m_closeButton;
//    QValueStack<QWidget*> *m_history;

};

#endif
