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
#include <kdevgenericfactory.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kaction.h>
#include <kurl.h>
#include <kparts/part.h>

#include "kdevpartcontroller.h"
#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "kdevplugininfo.h"
#include "commitdlg.h"
#include "execcommand.h"

static const KDevPluginInfo data("kdevperforce");

typedef KDevGenericFactory<PerforcePart> PerforceFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevperforce, PerforceFactory( data ) )

PerforcePart::PerforcePart( QObject *parent, const char *name, const QStringList & )
    : KDevVersionControl( &data, parent, name ? name : "PerforcePart" )
{
    setInstance(PerforceFactory::instance());
    setupActions();

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
}


PerforcePart::~PerforcePart()
{}

void PerforcePart::setupActions()
{
    actionEdit = new KAction( i18n("Edit"), 0, this, SLOT(slotActionEdit()),
                actionCollection(), "perforce_edit" );
    actionEdit->setToolTip(i18n("Edit"));
    actionEdit->setWhatsThis(i18n("<b>Edit</b><p>Opens file(s) in a client workspace for edit."));
    actionRevert = new KAction( i18n("Revert"), 0, this, SLOT(slotActionRevert()),
                actionCollection(), "perforce_revert" );
    actionRevert->setToolTip(i18n("Revert"));
    actionRevert->setWhatsThis(i18n("<b>Revert</b><p>Discards changes made to open files."));
    actionSubmit = new KAction( i18n("Submit"), 0, this, SLOT(slotActionCommit()),
                actionCollection(), "perforce_submit" );
    actionSubmit->setToolTip(i18n("Submit"));
    actionSubmit->setWhatsThis(i18n("<b>Submit</b><p>Sends changes made to open files to the depot."));
    actionSync = new KAction( i18n("Sync"), 0, this, SLOT(slotActionUpdate()),
                actionCollection(), "perforce_sync" );
    actionSync->setToolTip(i18n("Sync"));
    actionSync->setWhatsThis(i18n("<b>Sync</b><p>Copies files from the depot into the workspace."));
    actionDiff = new KAction( i18n("Diff Against Repository"), 0, this, SLOT(slotActionDiff()),
                actionCollection(), "perforce_diff" );
    actionDiff->setToolTip(i18n("Diff against repository"));
    actionDiff->setWhatsThis(i18n("<b>Diff against repository</b><p>Compares a client workspace file to a revision in the depot."));
    actionAdd = new KAction( i18n("Add to Repository"), 0, this, SLOT(slotActionAdd()),
                actionCollection(), "perforce_add" );
    actionAdd->setToolTip(i18n("Add to repository"));
    actionAdd->setWhatsThis(i18n("<b>Add to repository</b><p>Open file(s) in a client workspace for addition to the depot."));
    actionRemove = new KAction( i18n("Remove From Repository"), 0, this, SLOT(slotActionRemove()),
                actionCollection(), "perforce_remove" );
    actionRemove->setToolTip(i18n("Remove from repository"));
    actionRemove->setWhatsThis(i18n("<b>Remove from repository</b><p>Open file(s) in a client workspace for deletion from the depot."));
}

void PerforcePart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType( Context::FileContext )) {
        const FileContext *fcontext = static_cast<const FileContext*>(context);
        popupfile = fcontext->urls().first().fileName();
        QFileInfo fi( popupfile );
        popup->insertSeparator();

        KPopupMenu *sub = new KPopupMenu(popup);
        QString name = fi.fileName();
        sub->insertTitle( i18n("Actions for %1").arg(name) );

        int id = sub->insertItem( i18n("Edit"),
                           this, SLOT(slotEdit()) );
        sub->setWhatsThis(id, i18n("<b>Edit</b><p>Opens file(s) in a client workspace for edit."));
        id = sub->insertItem( i18n("Revert"),
                           this, SLOT(slotRevert()) );
        sub->setWhatsThis(id, i18n("<b>Revert</b><p>Discards changes made to open files."));
        id = sub->insertItem( i18n("Submit"),
                           this, SLOT(slotCommit()) );
        sub->setWhatsThis(id, i18n("<b>Submit</b><p>Sends changes made to open files to the depot."));
        id = sub->insertItem( i18n("Sync"),
                           this, SLOT(slotUpdate()) );
        sub->setWhatsThis(id, i18n("<b>Sync</b><p>Copies files from the depot into the workspace."));
        sub->insertSeparator();
        id = sub->insertItem( i18n("Diff Against Repository"),
                          this, SLOT(slotDiff()) );
        sub->setWhatsThis(id, i18n("<b>Diff against repository</b><p>Compares a client workspace file to a revision in the depot."));
        id = sub->insertItem( i18n("Add to Repository"),
                           this, SLOT(slotAdd()) );
        sub->setWhatsThis(id, i18n("<b>Add to repository</b><p>Open file(s) in a client workspace for addition to the depot."));
        id = sub->insertItem( i18n("Remove From Repository"),
                           this, SLOT(slotRemove()) );
        sub->setWhatsThis(id, i18n("<b>Remove from repository</b><p>Open file(s) in a client workspace for deletion from the depot."));
        id = popup->insertItem(i18n("Perforce"), sub);
    }
}

void PerforcePart::execCommand( const QString& cmd, const QString& filename )
{
    if ( filename.isEmpty() )
        return;

    QFileInfo fi( filename );
    if (fi.isDir()) {
        KMessageBox::error( 0, i18n("Cannot handle directories, please select single files") );
        return;
    }
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QString command("cd ");
    command += KProcess::quote(dir);
    command += " && p4 " + cmd + " ";
    command += name;

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);
}

void PerforcePart::edit( const QString& filename )
{
    execCommand( "edit", filename );
}

void PerforcePart::revert( const QString& filename )
{
    if ( KMessageBox::questionYesNo( 0,
            i18n("Do you really want to revert "
                 "the file %1 and lose all your changes?").arg( filename ) ) == KMessageBox::Yes ) {
        execCommand( "revert", filename );
    }
}

void PerforcePart::commit( const QString& filename )
{
    if ( filename.isEmpty() )
        return;

    QFileInfo fi( filename );
    if ( fi.isDir() ) {
        KMessageBox::error( 0, i18n("Submitting of subdirectories is not supported") );
        return;
    }

    CommitDialog d;
    QStringList lst;
    lst << filename;
    d.setFiles( lst );
    if (d.exec() == QDialog::Rejected)
        return;

    QString message = d.changeList();
    if (!message.isEmpty())
     message = KShellProcess::quote(message);

    QString command("echo " + message);
    command += " | p4 submit -i";

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand("", command);
}


void PerforcePart::update( const QString& filename )
{
    if ( filename.isEmpty() )
        return;

    QString dir, name;
    QFileInfo fi( filename );
    if (fi.isDir()) {
        dir = fi.absFilePath();
        name = "..."; // three dots means "recoursive"
    } else {
        dir = fi.dirPath();
        name = fi.fileName();
    }

    QString command("cd ");
    command += KProcess::quote(dir);
    command += " && p4 sync ";
    command += name;

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);
}


void PerforcePart::add( const QString& filename )
{
    execCommand( "add", filename );
}


void PerforcePart::remove( const QString& filename )
{
    execCommand( "delete", filename );
}

void PerforcePart::diff( const QString& filename )
{
    if ( filename.isEmpty() )
        return;

    QString name;
    QFileInfo fi( filename );

    if ( fi.isDir() ) {
	name = fi.absFilePath() + "...";
    } else {
	name = filename;
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
        KMessageBox::detailedError( 0, i18n("P4 output errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() ) {
        int s = KMessageBox::warningContinueCancelList( 0, i18n("P4 output errors during diff. Do you still want to continue?"),
                        QStringList::split( "\n", err, false ), i18n("Errors During Diff") );
        if ( s != KMessageBox::Continue )
            return;
    }

    if ( diff.isEmpty() ) {
        KMessageBox::information( 0, i18n("There is no difference to the repository."), i18n("No Differences Found") );
        return;
    }

    // strip the ==== headers
    static QRegExp rx( "(^|\\n)====.*====\\n" );
    rx.setMinimal( true );
    QString strippedDiff = diff;
    strippedDiff.replace( rx, QString::null );

    if (KDevDiffFrontend *diffFrontend = extension<KDevDiffFrontend>("KDevelop/DiffFrontend"))
        diffFrontend->showDiff( strippedDiff );
}

QString PerforcePart::currentFile()
{
    KParts::ReadOnlyPart *part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
    if ( part ) {
        KURL url = part->url();
        if ( url.isLocalFile() )
            return url.path();
    }
    return QString::null;
}

void PerforcePart::slotActionCommit()
{
    commit( currentFile() );
}

void PerforcePart::slotActionUpdate()
{
    update( currentFile() );
}
void PerforcePart::slotActionAdd()
{
    add( currentFile() );
}

void PerforcePart::slotActionRemove()
{
    remove( currentFile() );
}

void PerforcePart::slotActionEdit()
{
    edit( currentFile() );
}

void PerforcePart::slotActionRevert()
{
    revert( currentFile() );
}

void PerforcePart::slotActionDiff()
{
    diff( currentFile() );
}

void PerforcePart::slotCommit()
{
    commit( popupfile );
}

void PerforcePart::slotUpdate()
{
    update( popupfile );
}

void PerforcePart::slotAdd()
{
    add( popupfile );
}

void PerforcePart::slotRemove()
{
    remove( popupfile );
}

void PerforcePart::slotEdit()
{
    edit( popupfile );
}

void PerforcePart::slotRevert()
{
    revert( popupfile );
}

void PerforcePart::slotDiff()
{
    diff( popupfile );
}

#include "perforcepart.moc"
