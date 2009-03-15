/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
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

#ifndef CPPXMLPARSE
#define CPPXMLPARSE

#include "interfacecreator.h"
#include <QXmlStreamReader>
#include <QMap>

class XmlToKross : public InterfaceCreator
{
    public:
        XmlToKross(QXmlStreamReader& _xml) : xml(_xml) {}
        virtual ~XmlToKross() {}
        QXmlStreamReader& xml;
        QStringList definedClasses;
        QString inNamespace;
        method currentMethod;
        QMap <QString, QString> classNamespace;
        QStringList flags;
        int inclass;
        
        virtual int start();
};

#endif
