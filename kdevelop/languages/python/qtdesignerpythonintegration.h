/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef QTDESIGNERPYTHONINTEGRATION_H
#define QTDESIGNERPYTHONINTEGRATION_H

#include <qtdesignerintegration.h>

class QtDesignerPythonIntegration : public QtDesignerIntegration
{
Q_OBJECT
public:
    QtDesignerPythonIntegration(KDevLanguageSupport* part, ImplementationWidget* impl);

    ~QtDesignerPythonIntegration();

    virtual void addFunctionToClass(KInterfaceDesigner::Function function, ClassDom klass);

};

#endif
