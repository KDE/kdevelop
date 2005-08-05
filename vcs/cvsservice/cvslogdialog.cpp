/***************************************************************************
 *   Copyright (C) 200?-2003 by KDevelop Authors                           *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qvbox.h>
#include <qregexp.h>
#include <qdir.h>
#include <qstringlist.h>

#include <kmessagebox.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "cvsoptions.h"
#include "cvslogpage.h"
#include "cvsdiffpage.h"

#include "cvslogdialog.h"

///////////////////////////////////////////////////////////////////////////////
// class CVSLogDialog
///////////////////////////////////////////////////////////////////////////////

CVSLogDialog::CVSLogDialog( CvsService_stub *cvsService, QWidget *parent, const char *name, int )
    : KDialogBase( Tabbed, i18n("CVS Log & Diff Dialog"), Close, Close,
    parent, name? name : "logformdialog", false /*modal*/, true /*separator*/ ),
    m_cvsLogPage( 0 ), m_cvsService( cvsService )
{
    setWFlags( getWFlags() | WDestructiveClose );

    QVBox *vbox = addVBoxPage( i18n("Log From CVS") );
    m_cvsLogPage = new CVSLogPage( m_cvsService, vbox );

    connect( m_cvsLogPage, SIGNAL(diffRequested(const QString&, const QString&, const QString&)),
        this, SLOT(slotDiffRequested(const QString&, const QString&, const QString&)) );
}

///////////////////////////////////////////////////////////////////////////////

CVSLogDialog::~CVSLogDialog()
{
    kdDebug(9006) << "CVSLogDialog::~CVSLogDialog()" << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogDialog::startLog( const QString &workDir, const QString &pathName )
{
    kdDebug(9006) << "CVSLogDialog::start() here! workDir = " << workDir <<
        ", pathName = " << pathName << endl;

//    displayActionFeedback( true );
/*
    QVBox *vbox = addVBoxPage( i18n("Log From CVS: ") + pathName );
    m_cvsLogPage = new CVSLogPage( m_cvsService, vbox );
    this->resize( m_cvsLogPage->size() );

    connect( m_cvsLogPage, SIGNAL(linkClicked(const QString&, const QString&)),
        this, SLOT(slotDiffRequested(const QString&, const QString&)) );
*/
    m_cvsLogPage->startLog( workDir, pathName );
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogDialog::slotDiffRequested( const QString &pathName, const QString &revA, const QString &revB )
{
    kdDebug(9006) << "CVSLogDialog::slotDiffRequested()" << endl;

    // Create a new CVSDiffPage and start diffing process
    QString diffTitle =  i18n("Diff between %1 and %2").arg( revA ).arg( revB );
    QVBox *vbox = addVBoxPage( diffTitle );
    CVSDiffPage *diffPage = new CVSDiffPage( m_cvsService, vbox );
    diffPage->startDiff( pathName, revA, revB );
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogDialog::slotCancel()
{
    // Hmmm ...

    KDialogBase::slotCancel();
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogDialog::displayActionFeedback( bool working )
{
    if (working)
    {
        setCursor( KCursor::waitCursor() );
    }
    else
    {
        setCursor( KCursor::arrowCursor() );
    }
}

#include "cvslogdialog.moc"


