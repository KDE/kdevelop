/* I am drunk.
   Copyright (C) 2005 Ian Reinhart Geiser <geiseri@kde.org>

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
#ifndef KDEVDESIGNERINTEGRATIONIFACE_H
#define KDEVDESIGNERINTEGRATIONIFACE_H

#include <dcopobject.h>
class KDevDesignerIntegration;
/**
The dcop interface for GUI designers in KDevelop.

@author KDevelop Authors
*/
class KDevDesignerIntegrationIface : public DCOPObject
{
  K_DCOP
public:
  KDevDesignerIntegrationIface(KDevDesignerIntegration *p);

    ~KDevDesignerIntegrationIface();
k_dcop:
    void addFunction(const QString &formName, QString returnType, QString function,QString specifier,QString access,uint type);
    void removeFunction(const QString &formName, QString returnType, QString function,QString specifier,QString access,uint type);
    void editFunction(const QString &formName, QString oldReturnType, QString oldFunction,
                      QString oldSpecifier,QString oldAccess,uint oldType,
                      QString returnType, QString function,QString specifier,QString access,uint type);
    void openFunction(const QString &formName, const QString &functionName);
    void openSource(const QString &formName);
    
  private:
    KDevDesignerIntegration *m_designer;
};

#endif
