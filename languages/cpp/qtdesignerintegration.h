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
#ifndef QTDESIGNERINTEGRATION_H
#define QTDESIGNERINTEGRATION_H

#include <qmap.h>

#include <codemodel.h>
#include "kdevdesignerintegration.h"

class CppSupportPart;

class QtDesignerIntegration : public KDevDesignerIntegration
{
Q_OBJECT
public:
    QtDesignerIntegration(CppSupportPart *part, const char* name = 0);
    ~QtDesignerIntegration();

public slots:
    virtual void addFunction(const QString& formName, KInterfaceDesigner::Function function);
    virtual void editFunction(const QString& formName, KInterfaceDesigner::Function oldFunction, KInterfaceDesigner::Function function);
    virtual void removeFunction(const QString& formName, KInterfaceDesigner::Function function);
    
    virtual void saveSettings(QDomDocument dom, QString path);
    virtual void loadSettings(QDomDocument dom, QString path);

protected:
    bool selectImplementation(const QString &formName);
    void addFunctionToClass(KInterfaceDesigner::Function function, ClassDom klass);
    QString accessID(FunctionDom fun) const;
    
private:
    //Form file - derived class name 
    QMap<QString, ClassDom> m_implementations;
    
    CppSupportPart *m_part;
};

#endif
