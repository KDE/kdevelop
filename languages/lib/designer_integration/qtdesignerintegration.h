/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef QTDESIGNERINTEGRATION_H
#define QTDESIGNERINTEGRATION_H

#include <qmap.h>

#include <codemodel.h>
#include "kdevdesignerintegration.h"

class KDevLanguageSupport;
class ImplementationWidget;

/**
Qt Designer integration base class.
Contains language-independent implementation part of a @ref KDevDesignerIntegration interface.
Ready to use in KDevelop language support plugins.

Subclasses of this class should reimplement only pure virtual functions in the common case.
*/
class QtDesignerIntegration : public KDevDesignerIntegration
{
Q_OBJECT
public:
    QtDesignerIntegration(KDevLanguageSupport *part, ImplementationWidget *impl,
        bool classHasDefinitions, const char* name = 0);
    ~QtDesignerIntegration();

public slots:
    virtual void addFunction(const QString& formName, KInterfaceDesigner::Function function);
    virtual void editFunction(const QString& formName, KInterfaceDesigner::Function oldFunction, KInterfaceDesigner::Function function);
    virtual void removeFunction(const QString& formName, KInterfaceDesigner::Function function);

    virtual void openFunction(const QString &formName, const QString &functionName);
        
    virtual void saveSettings(QDomDocument dom, QString path);
    virtual void loadSettings(QDomDocument dom, QString path);

    bool selectImplementation(const QString &formName);
    
protected:
    /**Reimplement this to add a function to a class. This means you need to modify
    the source file and add actual code of a function.*/
    virtual void addFunctionToClass(KInterfaceDesigner::Function function, ClassDom klass) = 0;
    /**Modifies name to be a name of a implementation file for languages that have
    separate files for interface and implementation parts of a class. For example,
    C++ language support plugin will do:
    @code
    name.replace(".h", ".cpp");
    @endcode*/
    virtual void processImplementationName(QString &name);
    
    //Form file - derived class name 
    QMap<QString, ClassDom> m_implementations;
    
    KDevLanguageSupport *m_part;
    ImplementationWidget *m_impl;
    bool m_classHasDefinitions;
};

#endif
