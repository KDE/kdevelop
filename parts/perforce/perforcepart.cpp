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


void PerforcePart::slotCommit()
{
    KMessageBox::error( 0, "not implemented yet" );
/*  TODO: need a solution here...
    QString dir, name;
    QFileInfo fi(popupfile);
    if (fi.isDir()) {
        dir = fi.absFilePath();
        name = ".";
    } else {
        dir = fi.dirPath();
        name = fi.fileName();
    }
    
    CommitDialog d;
    if (d.exec() == QDialog::Rejected)
        return;

    QString message = d.logMessage();
    if (!message.isEmpty())
     message = KShellProcess::quote(message);

    QString command("cd ");
    command += dir;
    command += " && p4 submit -i ";
    command += name;
    command += "<\'" + message + "\'"

    makeFrontend()->queueCommand(dir, command);
*/
}


void PerforcePart::slotUpdate()
{
    QString dir, name;
    QFileInfo fi(popupfile);
    if (fi.isDir()) {
        dir = fi.absFilePath();
        name = ".";
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
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();
    
    QString command("cd ");
    command += dir;
    command += " && p4 add ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}


void PerforcePart::slotRemove()
{
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QString command("cd ");
    command += dir;
    command += " && p4 delete ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}

#include "perforcepart.moc"
