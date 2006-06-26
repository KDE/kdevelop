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
#ifndef IDEALMAINWINDOW_H
#define IDEALMAINWINDOW_H

#include <QList>
#include <QMainWindow>

#include "idealdefs.h"

namespace Ideal {

class ButtonBar;
class ToolView;

/**
@short Main Window for the Ideal UI.
*/
class IDEAL_EXPORT MainWindow: public QMainWindow {
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual void setAreaKind(int areaKind);

    virtual void addToolView(QWidget *view, Ideal::Place defaultPlace, int defaultAreaKind);
    virtual void removeToolView(QWidget *view);

    QList<ToolView*> toolViews() const;

protected:
//     ButtonBar *buttonBar(Ideal::Place place) const;

private:
    void initSettings();
    void initButtonBar(Ideal::Place place);

    struct MainWindowPrivate *d;

};

}

#endif
