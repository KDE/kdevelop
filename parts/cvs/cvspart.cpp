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
#include "cvswidget.h"

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
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "commitdlg.h"
#include "logform.h"
#include "execcommand.h"
#include "cvsoptionswidget.h"
#include "domutil.h"
#include "kdevtoplevel.h"

typedef KGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvs, CvsFactory( "kdevcvs" ) );

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & ) : KDevPlugin( parent, name ? name : "CvsPart" ),
        default_cvs("-f"),default_commit(""),default_update("-dP"),default_add(""),
default_remove("-f"),default_diff("-u3 -p"),default_log("") {
    setInstance(CvsFactory::instance());
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    m_widget = new CvsWidget(this);
    topLevel()->embedOutputView(m_widget, i18n("CVS"), i18n("cvs output"));
    
}

CvsPart::~CvsPart() {
    if (m_widget) topLevel()->removeView(m_widget); // Inform toplevel, that the output view is gone
    delete m_widget;
}


void CvsPart::contextMenu(QPopupMenu *popup, const Context *context) {
    if (context->hasType("file")) {
	const FileContext *fcontext = static_cast<const FileContext*>(context);
	popupfile = fcontext->fileName();
	QFileInfo fi(popupfile);
	popup->insertSeparator();
	
	KPopupMenu *sub = new KPopupMenu(popup);
	QString name = fi.fileName();
	sub->insertTitle( i18n("Actions for %1").arg(name) );
	sub->insertItem( i18n("Commit"),
			 this, SLOT(slotCommit()) );
	sub->insertItem( i18n("Update"),
			 this, SLOT(slotUpdate()) );
	sub->insertItem( i18n("Add to Repository"),
			 this, SLOT(slotAdd()) );
	sub->insertItem( i18n("Remove From Repository"),
			 this, SLOT(slotRemove()) );
	sub->insertSeparator();
	sub->insertItem( i18n("Diff Against Repository"),
			 this, SLOT(slotDiff()) );
	sub->insertItem( i18n("Log"),
			 this, SLOT(slotLog()) );
	
	popup->insertItem(i18n("CVS"), sub);
    }
}


void CvsPart::slotCommit() {
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
        message = " -m " + KShellProcess::quote(message);

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += dir;
    command += " && cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " commit ";
    command += DomUtil::readEntry(dom,"/kdevcvs/commitoptions",default_commit);
    command += " ";
    command += message;
    command += " ";
    command += name;

//    makeFrontend()->queueCommand(dir, command);
    m_widget->startCommand(dir,command);
}


void CvsPart::slotUpdate() {
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
    command += dir;
    command += " && cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " update ";
    command += DomUtil::readEntry(dom,"/kdevcvs/updateoptions",default_update);
    command += " ";
    command += name;

//    makeFrontend()->queueCommand(dir, command);
    m_widget->startCommand(dir,command);
}


void CvsPart::slotAdd() {
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += dir;
    command += " && cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " add ";
    command += DomUtil::readEntry(dom,"/kdevcvs/addoptions",default_add);
    command += " ";
    command += name;

//    makeFrontend()->queueCommand(dir, command);
    m_widget->startCommand(dir,command);
}


void CvsPart::slotRemove() {
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += dir;
    command += " && cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " remove ";
    command += DomUtil::readEntry(dom,"/kdevcvs/removeoptions",default_remove);
    command += " ";
    command += name;

//    makeFrontend()->queueCommand(dir, command);
    m_widget->startCommand(dir,command);
}

void CvsPart::slotLog() {
    LogForm* f = new LogForm();
    f->show();
    f->start( popupfile );
}

void CvsPart::slotDiff() {
    QFileInfo fi(popupfile);
    QString dir = fi.dirPath();
    QString name = fi.fileName();
    QStringList args;
    QString str;

    QDomDocument &dom = *this->projectDom();

    str = DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    
    if (str.length()) {
	QStringList list = QStringList::split(' ',str);
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) args << *it; 
    }
    args << "diff"; // cannot use "-u3 -p" since it will clash with ~/.cvsrc
    str = DomUtil::readEntry(dom,"/kdevcvs/diffoptions",default_diff);
    if (str.length()) {
	QStringList list = QStringList::split(' ',str);
	for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) args << *it; 
    }
    args << name;
    
    ExecCommand* cmv = new ExecCommand( "cvs", args, dir, this );
    connect( cmv, SIGNAL(finished( const QString&, const QString& )),
             this, SLOT(slotDiffFinished( const QString&, const QString& )) );
}

void CvsPart::slotDiffFinished( const QString& diff, const QString& err ) {
    if ( diff == QString::null && err == QString::null ) {
        kdDebug(9000) << "cvs diff cancelled" << endl;
        return; // user pressed cancel or an error occured
    }

    if ( diff.isEmpty() && !err.isEmpty() ) {
        KMessageBox::detailedError( 0, i18n("CVS outputted errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() ) {
        int s = KMessageBox::warningContinueCancelList( 0, i18n("CVS outputted errors during diff. Do you still want to continue?"),
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

void CvsPart::projectConfigWidget(KDialogBase *dlg) {
    QVBox *vbox = dlg->addVBoxPage(i18n("CVS"));
    CvsOptionsWidget *w = new CvsOptionsWidget(this, (QWidget *)vbox, "cvs config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

#include "cvspart.moc"
