/***************************************************************************
 *   Copyright (C) 2004 by SourceXtreme, Inc                               *
 *   oss@sourcextreme.com                                                  *
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
#include <qapplication.h>

#include "propertyeditor.h"
#include "propertylist.h"
#include "multiproperty.h"

using namespace PropertyLib;

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    PropertyEditor *editor = new PropertyEditor( 0 );

    PropertyList *currentList = new PropertyList();

    currentList->addProperty(
        new Property(Property::String, "Sample string",
                     "Sample description", "value" ) );
    currentList->addProperty(
        new Property(Property::Color, "Color",
                     "Sample color description", Qt::red ) );
    currentList->addProperty(
        new Property( Property::Pixmap, "Pixmap",
                      "sample pixmap description" ) );
    currentList->addProperty(
        new Property( Property::Rect, "Rectangle",
                      "sample rectangle", QRect( 10, 11, 30, 40 ) ) );
    currentList->addProperty(
        new Property( Property::Point, "Point",
                      "sample point", QPoint( 20, 30 ) ) );
    currentList->addProperty(
        new Property( Property::Boolean, "Boolean",
                      "sample bool", false ) );
    currentList->addProperty(
        new Property( Property::Integer, "Integer",
                      "sample integer", 7 ) );

    currentList->addProperty(
        new Property( Property::Double, "Double",
                      "sample double", 7.0 ) );

    QStringList things;
    things += "Thing 1";
    things += "Thing 2";

    currentList->addProperty(
       new Property( Property::StringList, "StringList",
                     "sample stringlist", things ) );

    editor->populateProperties( currentList );

    app.setMainWidget( editor );
    editor->show();

    return app.exec();
}

