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
#ifndef SHELLSCRIPTBUILDSYSTEM_H
#define SHELLSCRIPTBUILDSYSTEM_H

#include <kdevbuildsystem.h>

/**
@author KDevelop Authors
*/
class ShellScriptBuildSystem : public KDevBuildSystem
{
    Q_OBJECT
public:
    ShellScriptBuildSystem(QObject *parent = 0, const char *name = 0,  const QStringList args = QStringList());

    ~ShellScriptBuildSystem();

    virtual QString buildSystemName();
    virtual QValueList< ProjectConfigTab * > confWidgets();
    virtual void configureBuildItem(KDialogBase *dia, BuildBaseItem* it);
    virtual void createBuildSystem(BuildBaseItem* it);
    virtual void build(BuildBaseItem* it = 0);
    virtual void install(BuildBaseItem* it = 0);
    virtual void execute(BuildBaseItem* it = 0);
    virtual void clean(BuildBaseItem* it = 0);

};

#endif
