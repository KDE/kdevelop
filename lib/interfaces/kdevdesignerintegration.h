/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVDESIGNERINTEGRATION_H
#define KDEVDESIGNERINTEGRATION_H

#include <designer.h>
#include <qobject.h>
#include <qdom.h>

/**
@file kdevdesignerintegration.h
KDevelop designer integration interface.
*/

/**
KDevelop designer integration interface.
Designer integration object provides functionality which is used by integrated
into KDevelop GUI designers:
- adding/removing/editing functions (class methods);
- opening form implementation files;
- saving and loading designer integration settings.
.

Designer integration is a layer between visual form (GUI) designer of any type
and the current language support. Usually designers create some kind of forms
and allow developers to define actions which are executed upon some GUI events
(for example, Qt Designer allows to define slots connected to GUI signals).
The actual code with action implementations needs to be written by hand in source
files. Designer integration captures such implementation requests and opens
necessary files at desired line numbers and adds (edits, removes) code for implementations.

For example, Qt Designer integration should be able to create an subclass for a form
and reimplement slots defined in the form.
*/
class KDevDesignerIntegration : public QObject
{
Q_OBJECT
public:
    /**Constructor.
    @param parent Parent object.
    @param name Internal name.*/
    KDevDesignerIntegration(QObject *parent = 0, const char *name = 0);
    /**Destructor.*/
    ~KDevDesignerIntegration();
    
    /**Saves designer integration settings.*/
    virtual void saveSettings(QDomDocument dom, QString path) = 0;
    /**Loads designer integration settings.*/
    virtual void loadSettings(QDomDocument dom, QString path) = 0;
    
public slots:
    /**Adds the implementation for a function.
    @param formName The name of a GUI form.
    @param function The function which needs to be implemented in the source.*/
    virtual void addFunction(const QString &formName, KInterfaceDesigner::Function function) = 0;
    
    /**Removes the implementation of a function.
    @param formName The name of a GUI form.
    @param function The function which needs to be removed from the source.*/
    virtual void removeFunction(const QString &formName, KInterfaceDesigner::Function function) = 0;
    
    /**Edites the implementation of a function.
    @param formName The name of a GUI form.
    @param oldFunction The function which needs to be edited from the source.
    @param function The new function declaration.*/
    virtual void editFunction(const QString &formName, KInterfaceDesigner::Function oldFunction, KInterfaceDesigner::Function function) = 0;

    /**Opens the function and jumps to the line number of its definition (declaration).
    @param formName The name of a GUI form.
    @param functionName The name of a function to find in the source.*/
    virtual void openFunction(const QString &formName, const QString &functionName) = 0;
};

#endif
