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
#include <qregexp.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kapplication.h>


#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "commitdlg.h"
#include "execcommand.h"

typedef KGenericFactory<PerforcePart> PerforceFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevperforce, PerforceFactory( "kdevperforce" ) );

PerforcePart::PerforcePart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( "Perforce", "perforce", parent, name ? name : "PerforcePart" )
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
        sub->insertTitle( i18n("Actions for %1").arg(name) );
        sub->insertItem( i18n("Edit"),
                           this, SLOT(slotEdit()) );
        sub->insertItem( i18n("Revert"),
                           this, SLOT(slotRevert()) );
        sub->insertItem( i18n("Submit"),
                           this, SLOT(slotCommit()) );
        sub->insertItem( i18n("Sync"),
                           this, SLOT(slotUpdate()) );
        sub->insertSeparator();
        sub->insertItem( i18n("Diff Against Repository"),
			   this, SLOT(slotDiff()) );
        sub->insertItem( i18n("Add to Repository"),
                           this, SLOT(slotAdd()) );
        sub->insertItem( i18n("Remove From Repository"),
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

void PerforcePart::slotDiff()
{
    QString name;
    QFileInfo fi(popupfile);

    if ( fi.isDir() ) {
	name = fi.absFilePath() + "...";
    } else {
	name = popupfile;
    }
    QStringList args;

    args << "diff";
    args << name;
    ExecCommand* cmv = new ExecCommand( "p4", args, QString::null, QStringList(), this );
    connect( cmv, SIGNAL(finished( const QString&, const QString& )),
             this, SLOT(slotDiffFinished( const QString&, const QString& )) );
}

void PerforcePart::slotDiffFinished( const QString& diff, const QString& err )
{
    if ( diff.isNull() && err.isNull() ) {
        kdDebug(9000) << "p4 diff cancelled" << endl;
        return; // user pressed cancel or an error occured
    }

    if ( diff.isEmpty() && !err.isEmpty() ) {
        KMessageBox::detailedError( 0, i18n("P4 outputted errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() ) {
        int s = KMessageBox::warningContinueCancelList( 0, i18n("P4 outputted errors during diff. Do you still want to continue?"),
                        QStringList::split( "\n", err, false ), i18n("Errors During Diff") );
        if ( s != KMessageBox::Continue )
            return;
    }

    if ( diff.isEmpty() ) {
        KMessageBox::information( 0, i18n("There is no difference to the repository"), i18n("No differences found") );
        return;
    }

    // strip the ==== headers
    static QRegExp rx( "(^|\\n)====.*====\\n" );
    rx.setMinimal( true );
    QString strippedDiff = diff;
    strippedDiff.replace( rx, QString::null );

    Q_ASSERT( diffFrontend() );
    diffFrontend()->showDiff( strippedDiff );
}

#include "perforcepart.moc"
