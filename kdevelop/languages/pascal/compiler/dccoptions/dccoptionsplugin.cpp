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
#include <qvbox.h>
#include <qdialog.h>

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "dccoptionsplugin.h"

#include "optiontabs.h"

K_EXPORT_COMPONENT_FACTORY( libkdevdccoptions, KGenericFactory<DccOptionsPlugin>( "kdevdccoptions" ) )

DccOptionsPlugin::DccOptionsPlugin(QObject *parent, const char *name, const QStringList/* &args*/)
    : KDevCompilerOptions(parent, name)
{
}

DccOptionsPlugin::~DccOptionsPlugin()
{
}

QString DccOptionsPlugin::exec(QWidget *parent, const QString &flags)
{
    DccOptionsDialog *dlg = new DccOptionsDialog(parent, "dcc options dialog");
    QString newFlags = flags;
    dlg->setFlags(flags);
    if(dlg->exec() == QDialog::Accepted)
        newFlags = dlg->flags();
    delete dlg;
    return newFlags;
}


DccOptionsDialog::DccOptionsDialog( QWidget * parent, const char * name )
    : KDialogBase(Tabbed, i18n("Delphi Compiler Options"), Ok|Cancel, Ok, parent, name, true)
{
    QVBox *vbox;

    vbox = addVBoxPage(i18n("General"));
    general = new GeneralTab(vbox, "general tab");

    vbox = addVBoxPage(i18n("Locations I"));
    locations = new LocationsTab(vbox, "locations tab");

    vbox = addVBoxPage(i18n("Locations II"));
    locations2 = new Locations2Tab(vbox, "locations2 tab");

    vbox = addVBoxPage(i18n("Code Generation"));
    codegen = new CodegenTab(vbox, "codegen tab");

    vbox = addVBoxPage(i18n("Debug && Optimization"));
    debug_optim = new DebugOptimTab(vbox, "debug and optim tab");

    vbox = addVBoxPage(i18n("Linker"));
    linker = new LinkerTab(vbox, "linker tab");
}

DccOptionsDialog::~DccOptionsDialog( )
{
}

void DccOptionsDialog::setFlags( const QString & flags )
{
    QStringList flaglist = QStringList::split(" ", flags);

    general->readFlags(&flaglist);
    codegen->readFlags(&flaglist);
    debug_optim->readFlags(&flaglist);
    locations->readFlags(&flaglist);
    locations2->readFlags(&flaglist);
    linker->readFlags(&flaglist);

    unrecognizedFlags = flaglist;
}

QString DccOptionsDialog::flags( ) const
{
    QStringList flaglist;

    general->writeFlags(&flaglist);
    locations->writeFlags(&flaglist);
    locations2->writeFlags(&flaglist);
    codegen->writeFlags(&flaglist);
    debug_optim->writeFlags(&flaglist);
    linker->writeFlags(&flaglist);

    QString flags;
    QStringList::ConstIterator li;
    for (li = flaglist.begin(); li != flaglist.end(); ++li) {
        flags += (*li);
        flags += " ";
    }

    for (li = unrecognizedFlags.begin(); li != unrecognizedFlags.end(); ++li) {
        flags += (*li);
        flags += " ";
    }

    flags.truncate(flags.length()-1);
    return flags;
}

#include "dccoptionsplugin.moc"
