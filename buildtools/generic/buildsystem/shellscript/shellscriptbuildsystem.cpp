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
#include <kdialogbase.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <klocale.h>

#include "shellscriptbuildsystem.h"

#include "projectconfigurationwidget.h"

static const KAboutData data("kdevbuildscript", I18N_NOOP("Shell Script Build System"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevbuildscript, KDevGenericFactory<ShellScriptBuildSystem>( &data ) )

ShellScriptBuildSystem::ShellScriptBuildSystem(QObject *parent, const char *name, const QStringList )
 : KDevBuildSystem(parent, name)
{
}


ShellScriptBuildSystem::~ShellScriptBuildSystem()
{
}


QString ShellScriptBuildSystem::buildSystemName()
{
    return "ShellScript";
}

QValueList< ProjectConfigTab * > ShellScriptBuildSystem::confWidgets()
{
    QValueList<ProjectConfigTab *> configs;
    return configs;
}

void ShellScriptBuildSystem::configureBuildItem(KDialogBase *dia, BuildBaseItem* it)
{
    kdDebug() << "ShellScriptBuildSystem::configureBuildItem" << endl;
    if (!dia)
        return;
    kdDebug() << "ShellScriptBuildSystem::configureBuildItem : dia exists" << endl;
    KDevBuildSystem::configureBuildItem(dia, it);
    dia->exec();
}

void ShellScriptBuildSystem::createBuildSystem(BuildBaseItem* it)
{
}

void ShellScriptBuildSystem::build( BuildBaseItem * it )
{
}

void ShellScriptBuildSystem::install( BuildBaseItem * it )
{
}

void ShellScriptBuildSystem::execute( BuildBaseItem * it )
{
}

void ShellScriptBuildSystem::clean( BuildBaseItem * it )
{
}

#include "shellscriptbuildsystem.moc"
