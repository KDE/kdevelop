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
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "execcommand.h"
#include "domutil.h"
#include "kdevmainwindow.h"

typedef KGenericFactory<ClearcasePart> ClearcaseFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevclearcase, ClearcaseFactory( "kdevclearcase" ) );

ClearcasePart::ClearcasePart( QObject *parent, const char *name, const QStringList & )
        : KDevPlugin( "Clearcase", "clearcase", parent, name ? name : "ClearcasePart" ),
        default_checkin("-nc"),default_checkout("-unres -nc"),default_uncheckout("-rm"),
        default_create("-ci"),default_remove("-f"),default_diff("-pred -diff")
{
    setInstance(ClearcaseFactory::instance());
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

}

ClearcasePart::~ClearcasePart()
{}


void ClearcasePart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType("file")) {
        const FileContext *fcontext = static_cast<const FileContext*>(context);
        popupfile = fcontext->fileName();
        QFileInfo fi(popupfile);
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
        sub->insertItem( i18n("Create element"),
                         this, SLOT(slotCreate()) );
        sub->insertItem( i18n("Remove element"),
                         this, SLOT(slotRemove()) );
        sub->insertSeparator();
        sub->insertItem( i18n("Diff"),
                         this, SLOT(slotDiff()) );

        popup->insertItem(i18n("Clearcase"), sub);
    }
}

void ClearcasePart::slotCheckin()
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

    QDomDocument &dom = *this->projectDom();

    QString message = DomUtil::readEntry(dom,"/kdevclearcase/checkin_options",default_checkin);

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && ";
    command += " cleartool checkin ";
    command += message; // Already quoted, see above
    command += " ";
    command += KShellProcess::quote(name);

    makeFrontend()->queueCommand(dir, command);
}


void ClearcasePart::slotCheckout()
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

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && cleartool checkout ";
    command += DomUtil::readEntry(dom,"/kdevclearcase/checkout_options",default_checkout);
    command += " ";
    command += KShellProcess::quote(name);

    makeFrontend()->queueCommand(dir, command);
}


void ClearcasePart::slotUncheckout()
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

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dir);
    command += " && cleartool uncheckout ";
    command += DomUtil::readEntry(dom,"/kdevclearcase/uncheckout_options",default_uncheckout);
    command += " ";
    command += KShellProcess::quote(name);

    makeFrontend()->queueCommand(dir, command);
}

void ClearcasePart::slotCreate()
{
    QFileInfo fi(popupfile);
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

    makeFrontend()->queueCommand(dir, command);
}


void ClearcasePart::slotRemove()
{
    QFileInfo fi(popupfile);
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

    makeFrontend()->queueCommand(dir, command);
}


void ClearcasePart::slotDiff()
{
    QFileInfo fi(popupfile);
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
        kdDebug(9000) << i18n("clearcase diff cancelled") << endl;
        return; // user pressed cancel or an error occured
    }

    if ( diff.isEmpty() && !err.isEmpty() ) {
        KMessageBox::detailedError( 0, i18n("Clearcase outputted errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() ) {
        int s = KMessageBox::warningContinueCancelList( 0, i18n("Clearcase outputted errors during diff. Do you still want to continue?"),
                QStringList::split( "\n", err, false ), i18n("Errors during Diff") );
        if ( s != KMessageBox::Continue )
            return;
    }

    if ( diff.isEmpty() ) {
        KMessageBox::information( 0, i18n("There is no difference to the repository"), i18n("No Difference found") );
        return;
    }

    Q_ASSERT( diffFrontend() );
    diffFrontend()->showDiff( diff );
}

#include "clearcasepart.moc"
