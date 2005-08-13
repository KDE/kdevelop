/* This file is part of the KDE project
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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kdevdesignerintegrationiface.h"
#include <designer.h>
#include "kdevdesignerintegration.h"

KDevDesignerIntegrationIface::KDevDesignerIntegrationIface(KDevDesignerIntegration *p)
  : DCOPObject("GUIDesigner"), m_designer(p)
{
  //FIXME figure out what number our interface is...
}


KDevDesignerIntegrationIface::~KDevDesignerIntegrationIface()
{
}

void KDevDesignerIntegrationIface::addFunction( const QString & formName, QString returnType, QString function, QString specifier, QString access, uint type )
{
  KInterfaceDesigner::Function f = {returnType, function, specifier, access, (KInterfaceDesigner::FunctionType)type };
  m_designer->addFunction(formName, f);
}

void KDevDesignerIntegrationIface::removeFunction( const QString & formName, QString returnType, QString function, QString specifier, QString access, uint type )
{
  KInterfaceDesigner::Function f = {returnType, function, specifier, access, (KInterfaceDesigner::FunctionType)type };
  m_designer->addFunction(formName, f);
}

void KDevDesignerIntegrationIface::editFunction( const QString & formName, QString oldReturnType, QString oldFunction, QString oldSpecifier, QString oldAccess, uint oldType, QString returnType, QString function, QString specifier, QString access, uint type )
{
  KInterfaceDesigner::Function fold = {oldReturnType, oldFunction, oldSpecifier, oldAccess, (KInterfaceDesigner::FunctionType)oldType };
  KInterfaceDesigner::Function fnew = {returnType, function, specifier, access, (KInterfaceDesigner::FunctionType)type };
  m_designer->editFunction(formName,fold,fnew);
}

void KDevDesignerIntegrationIface::openFunction( const QString & formName, const QString & functionName )
{
  m_designer->openFunction(formName,functionName);
}

void KDevDesignerIntegrationIface::openSource( const QString & formName )
{
  m_designer->openSource(formName);
}


