/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef IMPLEMENTATIONWIDGET_H
#define IMPLEMENTATIONWIDGET_H

#include "implementationwidgetbase.h"

#include <codemodel.h>

class KListViewItem;
class CppSupportPart;

class ImplementationWidget : public CreateImplemenationWidgetBase
{
Q_OBJECT
public:
    ImplementationWidget(CppSupportPart *part, const QString &formName, QWidget* parent = 0, const char* name = 0, bool modal = FALSE);
    ~ImplementationWidget();
    /*$PUBLIC_FUNCTIONS$*/
    
    ClassDom selectedClass();

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/
    void processNamespaces(NamespaceDom dom, KListViewItem *parent);
    void processClasses(ClassDom dom, KListViewItem *parent);
    bool createClass();

protected slots:
    /*$PROTECTED_SLOTS$*/
    virtual void classNameChanged(const QString &text);
    virtual void accept();
    
private:
    CppSupportPart *m_part;
    ClassDom m_selectedClass;
    QString m_formName;
    QString m_baseClassName;
};

#endif

