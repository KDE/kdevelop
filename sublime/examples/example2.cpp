/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
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

#include <sublime/area.h>
#include <sublime/urldocument.h>
#include <sublime/controller.h>

#include <QApplication>
#include <QUrl>

#include "example2main.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Sublime::Controller *controller = new Sublime::Controller(&app);
    Sublime::Area *area = new Sublime::Area(controller, "Area");
    controller->addDefaultArea(area);
    Sublime::Document *doc = new Sublime::UrlDocument(controller, QUrl::fromLocalFile("~/foo.cpp"));
    area->addView(doc->createView());
    Example2Main *window = new Example2Main(controller);
    controller->showArea(area, window);
    window->resize(800, 600);
    window->show();

    return app.exec();
}

