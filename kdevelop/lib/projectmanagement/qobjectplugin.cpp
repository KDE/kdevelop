/***************************************************************************
                          qobjectplugin.cpp  -  description
                             -------------------
    begin                : Sun Jun 4 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qobjectplugin.h"

QObjectPlugin::QObjectPlugin(QObject *parent, const char *name ) : QObject(parent,name) {
}
QObjectPlugin::~QObjectPlugin(){
}

#include "qobjectplugin.moc"
