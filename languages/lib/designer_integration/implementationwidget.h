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

#ifndef IMPLEMENTATIONWIDGET_H
#define IMPLEMENTATIONWIDGET_H

#include "implementationwidgetbase.h"

#include <codemodel.h>

class KListViewItem;
class KDevLanguageSupport;

/**
Base class for implementation creation widgets.
Contains language-independent implementation widget that can be used
to create or select an implementation of a form in designer.

Implementations could be subclasses or simple files with callbacks, etc.

Subclasses of this class should reimplement only pure virtual functions in the common case.
*/
class ImplementationWidget : public CreateImplemenationWidgetBase
{
Q_OBJECT
public:
    ImplementationWidget(KDevLanguageSupport *part, QWidget* parent = 0, const char* name = 0, bool modal = false);
    virtual ~ImplementationWidget();
    /*$PUBLIC_FUNCTIONS$*/

    /**@returns The %DOM of selected (or created) class.*/    
    ClassDom selectedClass();
    
    /**Executes implementation widget for a form @p formName.*/
    int exec(const QString &formName);

public slots:
    /*$PUBLIC_SLOTS$*/

protected:    
    /*$PROTECTED_FUNCTIONS$*/
    /**Sets up the dialog. No need to reimplement unless you want to restrict
    the number of classes being displayed as possible implementation classes.*/
    void init(const QString &formName);
    
    void processNamespaces(NamespaceDom dom, KListViewItem *parent);
    void processClasses(ClassDom dom, KListViewItem *parent);
    
    /**Creates a class and adds it to a project. No need to reimplement.*/
    bool createClass();
    
    /**Reimplement to create actual files with the form implementation.
    @return The list of absolute paths to a files created.*/
    virtual QStringList createClassFiles() = 0;

protected slots:
    /*$PROTECTED_SLOTS$*/
    virtual void classNameChanged(const QString &text);
    virtual void accept();
    
protected:
    KDevLanguageSupport *m_part;
    ClassDom m_selectedClass;
    QString m_formName;
    QString m_baseClassName;
};

#endif

