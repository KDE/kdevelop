/*
    SPDX-FileCopyrightText: 2004 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    Based on code from Kopete
    SPDX-FileCopyrightText: 2002-2003 Martijn Klingens <klingens@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iplugincontroller.h"

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

