/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mksat.net>

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
#ifndef KINTERFACEDESIGNER_FORMEDITOR_H
#define KINTERFACEDESIGNER_FORMEDITOR_H

#include <kparts/part.h>

namespace KInterfaceDesigner{

enum DesignerType {QtDesigner, Glade};

enum FunctionType {ftFunction, ftQtSlot};

struct Function{
    QString returnType;
    QString function;
    QString specifier;
    QString access;
    FunctionType type;
};

class Designer: public KParts::ReadWritePart{
    Q_OBJECT
public:
    Designer(QObject *parent, const char *name);
    
    virtual void openProject(const QString &projectFile) = 0;
    virtual DesignerType designerType() = 0;

signals:
    void addedFunction(DesignerType type, const QString &formName, Function function);
    void removedFunction(DesignerType type, const QString &formName, Function function);
    void editedFunction(DesignerType type, const QString &formName, Function oldFunction, Function function);
    
};

}

#endif
