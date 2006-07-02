/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef IDEALTOOLVIEWWIDGET_H
#define IDEALTOOLVIEWWIDGET_H

#include <QDockWidget>

#include "idealdefs.h"

namespace Ideal {

/**
@short Toolview widget.

The dock widget that actually represents the toolview in the UI.
*/
class IDEAL_EXPORT ToolViewWidget: public QDockWidget {
    Q_OBJECT
public:
    ToolViewWidget(const QString &title, QWidget *parent = 0);
    ToolViewWidget(QWidget *parent = 0);


public slots:
    virtual void setVisible(bool v);

    void slotTopLevelChanged(bool topLevel);

signals:
    void visibilityChanged(bool v);

private:
    void init();

};

}

#endif
