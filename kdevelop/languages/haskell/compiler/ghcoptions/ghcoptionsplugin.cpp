/***************************************************************************
                          ghcoptionsplugin.cpp  -  description
                             -------------------
    begin                : Sat Aug 16 2003
    copyright            : (C) 2003 by Peter Robinson
    email                : listener@thaldyron.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include <qvbox.h>

#include <klocale.h>
#include <kgenericfactory.h>

//#include "optiontabs.h"
#include "ghcoptionsplugin.h"

K_EXPORT_COMPONENT_FACTORY( libkdevghcoptions, KGenericFactory<GhcOptionsPlugin>( "kdevghcoptions" ) )

GhcOptionsPlugin::GhcOptionsPlugin(QObject *parent, const char *name, const QStringList& /*args*/)
    : KDevCompilerOptions(parent, name)
{
}

GhcOptionsPlugin::~GhcOptionsPlugin()
{
}

QString GhcOptionsPlugin::exec( QWidget *parent, const QString &flags )
{
    GhcOptionsDialog *dlg = new GhcOptionsDialog(parent, "GHC Options Dialog");
    QString newFlags = flags;
    dlg->setFlags(flags);
    if (dlg->exec() == QDialog::Accepted)
        newFlags = dlg->flags();
    delete dlg;
    return newFlags;
}



GhcOptionsDialog::GhcOptionsDialog( QWidget *parent, const char *name )
    : KDialogBase(Tabbed, i18n("Glasgow Haskell Compiler"), Ok|Cancel, Ok, parent, name, true)
{
//    QVBox *vbox;

/*    vbox = addVBoxPage(i18n("Language"));
    language = new LanguageTab(vbox, "language tab");

    vbox = addVBoxPage(i18n("Locations I"));
    directories = new FilesAndDirectoriesTab(vbox, "directories tab");

    vbox = addVBoxPage(i18n("Locations II"));
    directories2 = new FilesAndDirectoriesTab2(vbox, "directories2 tab");

    vbox = addVBoxPage(i18n("Debug && Optimization"));
    debug_optim = new DebugOptimTab(vbox, "debug_optim tab");

    vbox = addVBoxPage(i18n("Code Generation"));
    codegen = new CodegenTab(vbox, "codegen tab");

    vbox = addVBoxPage(i18n("Assembler"));
    assembler = new AssemblerTab(vbox, "assembler tab");

    vbox = addVBoxPage(i18n("Linker"));
    linker = new LinkerTab(vbox, "linker tab");

    vbox = addVBoxPage(i18n("Feedback"));
    feedback = new FeedbackTab(vbox, "feedback tab");

    vbox = addVBoxPage(i18n("Miscellaneous"));
    misc = new MiscTab(vbox, "miscellaneous tab");*/
}

GhcOptionsDialog::~GhcOptionsDialog()
{
}

void GhcOptionsDialog::setFlags(const QString &flags)
{
    QStringList flaglist = QStringList::split(" ", flags);

/*    feedback->readFlags(&flaglist);
    language->readFlags(&flaglist);
    assembler->readFlags(&flaglist);
    linker->readFlags(&flaglist);
    codegen->readFlags(&flaglist);
    debug_optim->readFlags(&flaglist);
    directories->readFlags(&flaglist);
    directories2->readFlags(&flaglist);
    misc->readFlags(&flaglist);
    unrecognizedFlags = flaglist;*/
}

QString GhcOptionsDialog::flags() const
{
    QStringList flaglist;

 /*   language->writeFlags(&flaglist);
    directories->writeFlags(&flaglist);
    directories2->writeFlags(&flaglist);
    debug_optim->writeFlags(&flaglist);
    codegen->writeFlags(&flaglist);
    assembler->writeFlags(&flaglist);
    linker->writeFlags(&flaglist);
    feedback->writeFlags(&flaglist);
    misc->writeFlags(&flaglist);

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
    return flags;*/
return "";
}

#include "ghcoptionsplugin.moc"
