//
// C++ Implementation: cvslogdialog
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cvslogdialog.h"

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
    : KDialogBase( Tabbed, i18n("CVS Log & diff dialog"), Close, Close,
    parent, name? name : "logformdialog",false /*modal*/, true /*separator*/ ),
    m_cvsLogPage( 0 ), m_cvsService( cvsService )
{
    setWFlags( getWFlags() | WDestructiveClose );

    QVBox *page = addVBoxPage( QString("Dummy") );
    m_cvsLogPage = new CVSLogPage( m_cvsService, page );

    connect( m_cvsLogPage, SIGNAL(linkClicked(const QString&, const QString&)),
        this, SLOT(slotDiffRequested(const QString&, const QString&)) );
}

///////////////////////////////////////////////////////////////////////////////

CVSLogDialog::~CVSLogDialog()
{
    kdDebug() << "CVSLogDialog::~CVSLogDialog()" << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogDialog::startLog( const QString &workDir, const QString &pathName )
{
    kdDebug() << "CVSLogDialog::start() here! workDir = " << workDir <<
        ", pathName = " << pathName << endl;

//    displayActionFeedback( true );

    m_cvsLogPage->setCaption( pathName );
    m_cvsLogPage->startLog( workDir, pathName );
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogDialog::slotDiffRequested( const QString &revA, const QString &revB )
{
    // Create a new CVSDiffPage and start diffing process
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


