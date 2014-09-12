/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>              *
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
#include <k4aboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <KLocalizedString>

#include "example1main.h"

int main(int argc, char **argv)
{
    static const char description[] = "Sublime UI Library: Area Example";
    K4AboutData aboutData("example1", 0, ki18n("Example 1"),
                         "1.0", ki18n(description), K4AboutData::License_LGPL,
                         ki18n("(c) 2007, Alexander Dymo"), KLocalizedString(), "http://www.kdevelop.org" );

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    Example1Main *m = new Example1Main();
    m->show();

    return app.exec();
}

