/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Extended 2002 by Harald Fernengel <harry@kdevelop.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "perforcepart.h"

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kapplication.h>


#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "commitdlg.h"


typedef KGenericFactory<PerforcePart> PerforceFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevperforce, PerforceFactory( "kdevperforce" ) );

PerforcePart::PerforcePart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( parent, name )
{
    setInstance(PerforceFactory::instance());

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
}


PerforcePart::~PerforcePart()
{}


void PerforcePart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType("file")) {
        const FileContext *fcontext = static_cast<const FileContext*>(context);
        popupfile = fcontext->fileName();
        QFileInfo fi(popupfile);
	popup->insertSeparator();

	KPopupMenu *sub = new KPopupMenu(popup);
        QString name = fi.fileName();
        sub->insertItem( i18n("Edit: %1").arg(name),
                           this, SLOT(slotEdit()) );
        sub->insertItem( i18n("Revert: %1").arg(name),
                           this, SLOT(slotRevert()) );
        sub->insertItem( i18n("Submit: %1").arg(name),
                           this, SLOT(slotCommit()) );
        sub->insertItem( i18n("Sync: %1").arg(name),
                           this, SLOT(slotUpdate()) );
        sub->insertItem( i18n("Add to Repository: %1").arg(name),
                           this, SLOT(slotAdd()) );
        sub->insertItem( i18n("Remove From Repository: %1").arg(name),
                           this, SLOT(slotRemove()) );
	popup->insertItem(i18n("Perforce"), sub);
	
    }
}

void PerforcePart::execCommand( const QString& cmd ) 
{
    QFileInfo fi(popupfile);
    if (fi.isDir()) {
        KMessageBox::error( 0, i18n("Cannot handle directories, please select single files") );
        return;
    }
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QString command("cd ");
    command += dir;
    command += " && p4 " + cmd + " ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}

void PerforcePart::slotEdit()
{
    execCommand( "edit" );
}

void PerforcePart::slotRevert()
{
    if ( KMessageBox::questionYesNo( 0, 
            i18n("Do you really want to revert "
                 "the file %1 and lose all your changes?").arg(popupfile) ) == KMessageBox::Yes ) {
        execCommand( "revert" );
    }
}

void PerforcePart::slotCommit()
{
    QFileInfo fi(popupfile);
    if (fi.isDir()) {
        KMessageBox::error( 0, i18n("Submitting of subdirectories is not supported") );
        return;
    }
    
    CommitDialog d;
    QStringList lst;
    lst << popupfile;
    d.setFiles( lst );
    if (d.exec() == QDialog::Rejected)
        return;

    QString message = d.changeList();
    if (!message.isEmpty())
     message = KShellProcess::quote(message);

    QString command("echo " + message);
    command += " | p4 submit -i";

    makeFrontend()->queueCommand("", command);
}


void PerforcePart::slotUpdate()
{
    QString dir, name;
    QFileInfo fi(popupfile);
    if (fi.isDir()) {
        dir = fi.absFilePath();
        name = "..."; // three dots means "recoursive"
    } else {
        dir = fi.dirPath();
        name = fi.fileName();
    }

    QString command("cd ");
    command += dir;
    command += " && p4 sync ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}


void PerforcePart::slotAdd()
{
    execCommand( "add" );
}


void PerforcePart::slotRemove()
{
    execCommand( "delete" );
}

#include "perforcepart.moc"
