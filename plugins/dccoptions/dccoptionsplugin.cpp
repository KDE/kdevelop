/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <klocale.h>
#include <kgenericfactory.h>

#include "dccoptionsplugin.h"

K_EXPORT_COMPONENT_FACTORY( libkdevdccoptions, KGenericFactory<DccOptionsPlugin>( "kdevdccoptions" ) );

DccOptionsPlugin::DccOptionsPlugin(QObject *parent, const char *name, const QStringList &args)
    : KDevCompilerOptions(parent, name)
{
}

DccOptionsPlugin::~DccOptionsPlugin()
{
}

QString DccOptionsPlugin::exec(QWidget *parent, const QString &flags)
{
    return flags;
}

#include "dccoptionsplugin.moc"
