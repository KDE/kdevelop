/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Hamish Rodda                                    *
 *   rodda@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "debuggerconfigwidget.h"

#include "debuggerplugin.h"

#include <kurlrequester.h>

#include <kgenericfactory.h>
#include <KConfigDialogManager>

#include "debuggerconfig.h"

namespace GDBDebugger
{

K_PLUGIN_FACTORY(DebuggerConfigFactory, registerPlugin<DebuggerConfigWidget>();)
K_EXPORT_PLUGIN(DebuggerConfigFactory("kcm_kdev_cppdebugger"))

DebuggerConfigWidget::DebuggerConfigWidget(QWidget *parent, const QVariantList &args)
    : KCModule( DebuggerConfigFactory::componentData(), parent, args )
{
    setupUi(this);

    kcfg_gdbPath->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);

    addConfig( DebuggerConfig::self(), this );

    load();

//     KConfigGroup config(KGlobal::config(), "GDB Debugger");
//     QString shell = config.readEntry("dbgshell","no_value");
//     if( shell == QString("no_value") )
//     {
//         QFileInfo info( part->project()->buildDirectory() + "/libtool" );
//         if( info.exists() ) {
//             shell = "libtool";
//         } else {
//             // Try one directory up.
//             info.setFile( part->project()->buildDirectory() + "/../libtool" );
//             if( info.exists() ) {
//                 shell = "../libtool";
//             } else {
//                 // Give up.
//                 shell = QString::null;
//             }
//         }
//     }
//     kcfg_debuggingShell->setUrl( shell );
}


DebuggerConfigWidget::~DebuggerConfigWidget()
{}

}

#include "debuggerconfigwidget.moc"
