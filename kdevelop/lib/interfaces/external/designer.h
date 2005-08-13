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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KINTERFACEDESIGNER_FORMEDITOR_H
#define KINTERFACEDESIGNER_FORMEDITOR_H

#include <kparts/part.h>

/**Contains KInterfaceDesigner classes.*/
namespace KInterfaceDesigner{

/**The type of a GUI Designer.*/
enum DesignerType {
    QtDesigner    /**<Qt Designer.*/,
    Glade         /**<Glade (version >= 3).*/
};

/**Function type.*/
enum FunctionType {
    ftFunction    /**<Simple function or a callback.*/,
    ftQtSlot      /**<Qt slot.*/
};

/**Function.*/
struct Function{
    /**Return type.*/
    QString returnType;
    /**Function name.*/
    QString function;
    /**Specifier, e.g. virtual, static, etc.*/
    QString specifier;
    /**Access, e.g. private, protected, public, etc.*/
    QString access;
    /**Function type.*/
    FunctionType type;
};

/**
GUI Designer Part Interface.
Each KPart that wants to act as a GUI Designer must implement this interface.
It defines necessary signals to communicate with an IDE and abstract virtual
functions to determine designer type.

Parts that implement this interface must emit its signals when necessary.
See signals documentation for an explanation on when to emit those.

If a part which can "design" user interface files of a certain mimetype
implements this interface and sets itself as a default handler for that 
mimetype then it becomes automatically integrated into KDevelop IDE.
*/
class Designer: public KParts::ReadWritePart{
    Q_OBJECT
public:
    Designer(QObject *parent, const char *name);
    
    /**Reimplement this to be able to open projects.*/
    virtual void openProject(const QString &projectFile) = 0;
    /**Reimplement this return the type of this designer.*/
    virtual DesignerType designerType() = 0;

signals:
    /**Emit this signal when a function was added by a designer. For example, when a slot 
    or a callback function was defined.*/
    void addedFunction(DesignerType type, const QString &formName, Function function);
    /**Emit this signal when a function was removed by a designer.*/
    void removedFunction(DesignerType type, const QString &formName, Function function);
    /**Emit this signal when a function signature was edited by a designer.*/
    void editedFunction(DesignerType type, const QString &formName, Function oldFunction, Function function);
    
    /**Emit this signal when a designer wants to open the editor with function definition.*/
    void editFunction(DesignerType type, const QString &formName, const QString &functionName);
    /**Emit this signal when a designer wants to open the editor for a form sources.*/
    void editSource(DesignerType type, const QString &formName);
    
    /**Emitted when a form state is changed in the designer.
    @param formName An absolute name of the form file.
    @param status 0: form is clean, 1: form is modified.*/
    void newStatus(const QString &formName, int status);
};

}

#endif
