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

#include <klocale.h>
#include <kgenericfactory.h>

#include "optiontabs.h"
#include "fpcoptionsplugin.h"

K_EXPORT_COMPONENT_FACTORY( libkdevfpcoptions, KGenericFactory<FpcOptionsPlugin>( "kdevfpcoptions" ) );

FpcOptionsPlugin::FpcOptionsPlugin(QObject *parent, const char *name, const QStringList &args)
    : KDevCompilerOptions(parent, name)
{
}

FpcOptionsPlugin::~FpcOptionsPlugin()
{
}

QString FpcOptionsPlugin::exec(QWidget *parent, const QString &flags)
{
    FpcOptionsDialog *dlg = new FpcOptionsDialog(parent, "fpc options dialog");
    dlg->setFlags(flags);
    dlg->exec();
    QString newFlags = dlg->flags();
    delete dlg;
    return newFlags;
}



FpcOptionsDialog::FpcOptionsDialog( QWidget *parent, const char *name )
    : KDialogBase(Tabbed, i18n("Free Pascal Compiler Options"), Ok|Cancel, Ok, parent, name, true)
{
    QVBox *vbox;

    vbox = addVBoxPage(i18n("Files and Directories"));
    directories = new FilesAndDirectoriesTab(vbox, "directories tab");

    vbox = addVBoxPage(i18n("Language"));
    language = new LanguageTab(vbox, "language tab");

    vbox = addVBoxPage(i18n("Feedback"));
    feedback = new FeedbackTab(vbox, "feedback tab");
}

FpcOptionsDialog::~FpcOptionsDialog()
{
}

void FpcOptionsDialog::setFlags(const QString &flags)
{
    QStringList flaglist = QStringList::split(" ", flags);

    feedback->readFlags(&flaglist);
    language->readFlags(&flaglist);
    directories->readFlags(&flaglist);
    unrecognizedFlags = flaglist;
}

QString FpcOptionsDialog::flags() const
{
    QStringList flaglist;

    directories->writeFlags(&flaglist);
    language->writeFlags(&flaglist);
    feedback->writeFlags(&flaglist);

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
