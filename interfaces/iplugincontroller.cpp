/* This file is part of the KDE project
Copyright 2004 Alexander Dymo <adymo@kdevelop.org>
Copyright     2006 Matt Rogers <mattr@kde.org
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

Based on code from Kopete
Copyright 2002-2003 Martijn Klingens <klingens@kde.org>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "iplugincontroller.h"

#include <kservicetypetrader.h>
#include "ipluginversion.h"

namespace KDevelop
{

IPluginController::IPluginController( QObject* parent )
: QObject( parent )
{
}


IPluginController::~IPluginController()
{
}

}

#include "iplugincontroller.moc"
