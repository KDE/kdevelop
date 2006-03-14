/***************************************************************************
 *   Copyright (C) 2003 by Ajay Guleria                                    *
 *   ajay_guleria at yahoo dot com                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "clearcasepart.h"
#include "commentdlg.h"

#include <qfileinfo.h>
#include <qpopupmenu.h>

#include <kpopupmenu.h>
#include <kdebug.h>
#include <kdevgenericfactory.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "execcommand.h"
#include "domutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevplugininfo.h"

#include "clearcasefileinfoprovider.h"
#include "clearcasemanipulator.h"


static const KDevPluginInfo data("kdevclearcase");

typedef KDevGenericFactory<ClearcasePart> ClearcaseFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevclearcase, ClearcaseFactory( data ) )

ClearcasePart::ClearcasePart( QObject *parent, const char *name, const QStringList & )
        : KDevVersionControl( &data, parent, name ? name : "ClearcasePart" ),
          default_checkin(""),
          default_checkout(""),
          default_uncheckout("-rm"),
          default_create("-ci"),
          default_remove("-f"),
          default_lshistory(""),
          default_diff("-pred -diff"),
          default_lscheckout("-recurse")
{

    // check if project directory is valid and cache it
    isValidCCDirectory_ = ClearcaseManipulator::isCCRepository( project()->projectDirectory() );

    fileInfoProvider_ = new ClearcaseFileinfoProvider(this);

    setInstance(ClearcaseFactory::instance());
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
}

ClearcasePart::~ClearcasePart()
{}



bool ClearcasePart::isValidDirectory(const QString &dirPath) const {
    return isValidCCDirectory_;
}


void ClearcasePart::contextMenu(QPopupMenu *popup, const Context *context)
{

    if (context->hasType( Context::FileContext )) {
        const FileContext *fcontext = static_cast<const FileContext*>(context);
        popupfile_ = fcontext->urls().first().path();

        QFileInfo fi(popupfile_);
        popup->insertSeparator();

        KPopupMenu *sub = new KPopupMenu(popup);
        QString name = fi.fileName();
        sub->insertTitle( i18n("Actions for %1").arg(name) );
        sub->insertItem( i18n("Checkin"),
                         this, SLOT(slotCheckin()) );
        sub->insertItem( i18n("Checkout"),
                         this, SLOT(slotCheckout()) );
        sub->insertItem( i18n("Uncheckout"),
                         this, SLOT(slotUncheckout()) );
        sub->insertSeparator();
        sub->insertItem( i18n("Create Element"),
                         this, SLOT(slotCreate()) );
        sub->insertItem( i18n("Remove Element"),
                         this, SLOT(slotRemove()) );
        sub->insertSeparator();
        sub->insertItem( i18n("History"),
                         this, SLOT(slotListHistory()) );
        sub->insertSeparator();
        sub->insertItem( i18n("Diff"),
                         this, SLOT(slotDiff()) );

        sub->insertSeparator();
        sub->insertItem( i18n("List Checkouts"),
                         this, SLOT(slotListCheckouts()) );

        popup->insertItem(i18n("Clearcase"), sub);

	if (!project() || !isValidDirectory( project()->projectDirectory() )) {
	    sub->setEnabled( false );
	}
    }
}


void ClearcasePart::slotCheckin()
{
    QString dir, name;
    QFileInfo fi(popupfile_);
    dir = fi.dirPath();
    name = fi.fileName();

    CcaseCommentDlg dlg(FALSE);
    if (dlg.exec() == QDialog::Rejected)
        return;

    QDomDocument &dom = *this->projectDom();
    QString message = DomUtil::readEntry(dom,"/kdevclearcase/checkin_options",default_checkin);
    if(dlg.logMessage().isEmpty())
        message += "-nc ";
    else
        message += "-c \"" + dlg.logMessage() + "\"";

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && ";
    command += " cleartool checkin ";
    command += message; // Already quoted, see above
    command += " ";
    command += KShellProcess::quote(name);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);
}


void ClearcasePart::slotCheckout()
{
    QString dir, name;
    QFileInfo fi(popupfile_);
    dir = fi.dirPath();
    name = fi.fileName();

    CcaseCommentDlg dlg(TRUE);
    if (dlg.exec() == QDialog::Rejected)
        return;

    QDomDocument &dom = *this->projectDom();
    QString message = DomUtil::readEntry(dom,"/kdevclearcase/checkout_options",default_checkout);
    if(!dlg.isReserved())
       message += "-unres ";
    if(dlg.logMessage().isEmpty())
       message += "-nc ";
    else
       message += "-c \"" + dlg.logMessage() + "\"";

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && cleartool checkout ";
    command += message;
    command += " ";
    command += KShellProcess::quote(name);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);

    emit finishedFetching(dir);
}


void ClearcasePart::slotUncheckout()
{
    QString dir, name;
    QFileInfo fi(popupfile_);
    dir = fi.dirPath();
    name = fi.fileName();

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && cleartool uncheckout ";
    command += DomUtil::readEntry(dom,"/kdevclearcase/uncheckout_options",default_uncheckout);
    command += " ";
    command += KShellProcess::quote(name);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);

    emit finishedFetching(dir);
}

void ClearcasePart::slotCreate()
{
    QFileInfo fi(popupfile_);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QDomDocument &dom = *this->projectDom();

    // Checking whether current directory is checked out or not is cumbersome so skip it for now
    QString command("cd ");
    command += KShellProcess::quote(dir);
    QFileInfo di(dir);
    if(!di.isWritable()) { // Work-around to check if directory is checked out
        command += " && cleartool co -unres -nc ";
        command += KShellProcess::quote(dir);
    }
    command += " && cleartool mkelem ";
    if(fi.isDir())
        command += " -elt directory ";
    command += DomUtil::readEntry(dom,"/kdevclearcase/create_options",default_create);
    command += " ";
    command += KShellProcess::quote(name);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);

    emit finishedFetching(dir);
}


void ClearcasePart::slotRemove()
{
    QFileInfo fi(popupfile_);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dir);
    QFileInfo di(dir);
    if(!di.isWritable()) { // Work-around to check if directory is checked out
        command += " && cleartool co -unres -nc ";
        command += KShellProcess::quote(dir);
    }
    command += " && cleartool rmname "; // Don't use rm command
    command += DomUtil::readEntry(dom,"/kdevclearcase/remove_options",default_remove);
    command += " ";
    command += KShellProcess::quote(name);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);

    emit finishedFetching(dir);
}

void ClearcasePart::slotListHistory()
{
    QFileInfo fi(popupfile_);
    QString dir = fi.dirPath();
    QString name = fi.fileName();
    QStringList args;
    QStringList env;
    QString str;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && cleartool lshistory ";
    command += DomUtil::readEntry(dom, "/kdevclearcase/lshistory_options", default_lshistory);
    command += " ";
    command += KShellProcess::quote(name);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);
}

void ClearcasePart::slotDiff()
{
    QFileInfo fi(popupfile_);
    QString dir = fi.dirPath();
    QString name = fi.fileName();
    QStringList args;
    QStringList env;
    QString str;

    QDomDocument &dom = *this->projectDom();

    args << "diff";
    str = DomUtil::readEntry(dom,"/kdevclearcase/diff_options",default_diff);
    if (str.length()) {
        QStringList list = QStringList::split(' ',str);
        for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) args << *it;
    }

    args << name;

    ExecCommand* cmv = new ExecCommand( "cleartool", args, dir, env, this );
    connect( cmv, SIGNAL(finished( const QString&, const QString& )),
             this, SLOT(slotDiffFinished( const QString&, const QString& )) );
}


void ClearcasePart::slotDiffFinished( const QString& diff, const QString& err )
{
    if ( diff.isNull() && err.isNull() ) {
        kdDebug(9000) << "clearcase diff canceled" << endl;
        return; // user pressed cancel or an error occured
    }

    if ( diff.isEmpty() && !err.isEmpty() ) {
        KMessageBox::detailedError( 0, i18n("Clearcase output errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() ) {
        int s = KMessageBox::warningContinueCancelList( 0, i18n("Clearcase outputted errors during diff. Do you still want to continue?"),
                QStringList::split( "\n", err, false ), i18n("Errors During Diff") );
        if ( s != KMessageBox::Continue )
            return;
    }

    if ( diff.isEmpty() ) {
        KMessageBox::information( 0, i18n("There is no difference to the repository."), i18n("No Difference Found") );
        return;
    }

    if (KDevDiffFrontend *diffFrontend = extension<KDevDiffFrontend>("KDevelop/DiffFrontend"))
        diffFrontend->showDiff( diff );
}

void ClearcasePart::slotListCheckouts()
{
    QString dir;
    QFileInfo fi(popupfile_);
    if (fi.isDir()) {
        dir = fi.absFilePath();
    } else {
        dir = fi.dirPath();
    }

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && cleartool lsco ";
    command += DomUtil::readEntry(dom, "/kdevclearcase/lscheckout_options", default_lscheckout);

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, command);

}


#include "clearcasepart.moc"
