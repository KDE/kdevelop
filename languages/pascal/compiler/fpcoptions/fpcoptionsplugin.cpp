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
#include <q3vbox.h>

#include <klocale.h>
#include <kgenericfactory.h>

#include "optiontabs.h"
#include "fpcoptionsplugin.h"

K_EXPORT_COMPONENT_FACTORY( libkdevfpcoptions, KGenericFactory<FpcOptionsPlugin>( "kdevfpcoptions" ) )

FpcOptionsPlugin::FpcOptionsPlugin(QObject *parent, const char *name, const QStringList& /*args*/)
    : KDevCompilerOptions(parent, name)
{
}

FpcOptionsPlugin::~FpcOptionsPlugin()
{
}

QString FpcOptionsPlugin::exec(QWidget *parent, const QString &flags)
{
    FpcOptionsDialog *dlg = new FpcOptionsDialog(parent, "fpc options dialog");
    QString newFlags = flags;
    dlg->setFlags(flags);
    if (dlg->exec() == QDialog::Accepted)
        newFlags = dlg->flags();
    delete dlg;
    return newFlags;
}



FpcOptionsDialog::FpcOptionsDialog( QWidget *parent, const char *name )
    : KDialogBase(Tabbed, i18n("Free Pascal Compiler Options"), Ok|Cancel, Ok, parent, name, true)
{
    Q3VBox *vbox;

    vbox = addVBoxPage(i18n("Language"));
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
    misc = new MiscTab(vbox, "miscellaneous tab");
}

FpcOptionsDialog::~FpcOptionsDialog()
{
}

void FpcOptionsDialog::setFlags(const QString &flags)
{
    QStringList flaglist = QStringList::split(" ", flags);

    feedback->readFlags(&flaglist);
    language->readFlags(&flaglist);
    assembler->readFlags(&flaglist);
    linker->readFlags(&flaglist);
    codegen->readFlags(&flaglist);
    debug_optim->readFlags(&flaglist);
    directories->readFlags(&flaglist);
    directories2->readFlags(&flaglist);
    misc->readFlags(&flaglist);
    unrecognizedFlags = flaglist;
}

QString FpcOptionsDialog::flags() const
{
    QStringList flaglist;

    language->writeFlags(&flaglist);
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
    return flags;
}

#include "fpcoptionsplugin.moc"
