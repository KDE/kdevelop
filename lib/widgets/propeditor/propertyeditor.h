/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <qdockwindow.h>
#include <qwidget.h>
#include <qstring.h>
#include <qmap.h>

#include "property.h"

class QWidget;
class QTable;
class QString;
/**
  *@author Alexander Dymo
  Docked window that contains property editor - QTable with customized editors
  */

class PropertyEditor : public QDockWindow  {
   Q_OBJECT
public:
    PropertyEditor( Place p = InDock, QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    ~PropertyEditor();

public slots:
    void populateProperties(QMap<QString, PropPtr> *v_props);
    void clearProperties();

    void emitPropertyChange(QString name, QVariant newValue);

private:
    QTable *table;
    QMap<QString, PropPtr> *props;

signals:
    /**Signals the change of "name" property value to "newValue" */
    void propertyChanged(QString name, QVariant newValue);
};

#endif
