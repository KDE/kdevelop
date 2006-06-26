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

#include <QApplication>
#include <QWidget>
#include <QLayout>
#include <QMainWindow>
#include <QToolBar>
#include <QTextEdit>
#include <QDockWidget>

#include "area.h"
#include "button.h"
#include "buttonbar.h"

#include "mainwindow.h"

using namespace Ideal;


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;

    for (int i = 0; i < 3; ++i)
    {
        QWidget *tv = new QTextEdit(0);
        tv->setWindowTitle("Text Edit");
        w.addToolView(tv, Ideal::Left, Area::Code);
    }

    w.setAreaKind(Area::Code);
    w.resize(800, 600);
    w.show();

    return app.exec();
}
