/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvspart.h"

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kprocess.h>

#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "commitdlg.h"
#include "logform.h"


typedef KGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvs, CvsFactory( "kdevcvs" ) );

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( parent, name )
{
    setInstance(CvsFactory::instance());

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
}


CvsPart::~CvsPart()
{}


void CvsPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType("file")) {
        const FileContext *fcontext = static_cast<const FileContext*>(context);
        popupfile = fcontext->fileName();
        QFileInfo fi(popupfile);
	popup->insertSeparator();

	KPopupMenu *sub = new KPopupMenu(popup);
        QString name = fi.fileName();
        sub->insertItem( i18n("Commit: %1").arg(name),
                           this, SLOT(slotCommit()) );
        sub->insertItem( i18n("Update: %1").arg(name),
                           this, SLOT(slotUpdate()) );
        sub->insertItem( i18n("Add to Repository: %1").arg(name),
                           this, SLOT(slotAdd()) );
        sub->insertItem( i18n("Remove From Repository: %1").arg(name),
                           this, SLOT(slotRemove()) );
        sub->insertSeparator();
        sub->insertItem( i18n("Log: %1").arg(name),
                           this, SLOT(slotLog()) );
	popup->insertItem(i18n("CVS"), sub);

    }
}


void CvsPart::slotCommit()
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

    CommitDialog d;
    if (d.exec() == QDialog::Rejected)
        return;

    QString message = d.logMessage();
    if (!message.isEmpty())
     message = KShellProcess::quote(message);

    QString command("cd ");
    command += dir;
    command += " && cvs commit -m ";
    command += message;
    command += " ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}


void CvsPart::slotUpdate()
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
    command += " && cvs update -dP ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}


void CvsPart::slotAdd()
{
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QString command("cd ");
    command += dir;
    command += " && cvs add ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}


void CvsPart::slotRemove()
{
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QString command("cd ");
    command += dir;
    command += " && cvs remove -f ";
    command += name;

    makeFrontend()->queueCommand(dir, command);
}

void CvsPart::slotLog()
{
    LogForm* f = new LogForm();
    f->show();
    f->start( popupfile );
}

#include "cvspart.moc"
