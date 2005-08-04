/***************************************************************************
 *   Copyright (C) 2005 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <q3vbox.h>
#include <qdir.h>
#include <qstringlist.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "annotatedialog.h"
#include "annotatepage.h"

AnnotateDialog::AnnotateDialog( CvsService_stub *cvsService, QWidget *parent, const char *name, int )
    : KDialogBase( Tabbed, i18n("CVS Annotate Dialog"), Close, Close,
    parent, name? name : "annotateformdialog", false /*modal*/, true /*separator*/ ),
    m_cvsService( cvsService )
{
    setWFlags( getWFlags() | WDestructiveClose );

    Q3VBox *vbox = addVBoxPage( i18n("Annotate") );
    m_cvsAnnotatePage = new AnnotatePage( m_cvsService, vbox );

    connect( m_cvsAnnotatePage, SIGNAL(requestAnnotate(const QString)),
             this, SLOT(slotAnnotate(const QString)) );
}

AnnotateDialog::~AnnotateDialog()
{
    kdDebug(9006) << "AnnotateDialog::~AnnotateDialog()" << endl;
}

void AnnotateDialog::startFirstAnnotate( const QString pathName, const QString revision )
{
    kdDebug(9006) << "AnnotateDialog::startFirstAnnotate() pathName = " << pathName << 
            "revision = " << revision << endl;

    //save the filename for any later use
    m_pathName = pathName;

    m_cvsAnnotatePage->startAnnotate( pathName, revision );
}

void AnnotateDialog::slotAnnotate(const QString rev)
{
    kdDebug(9006) << "AnnotateDialog::slotAnnotate(QString) revision = " << rev << endl;

    Q3VBox *vbox = addVBoxPage( i18n("Annotate")+" "+rev );
    AnnotatePage * page = new AnnotatePage( m_cvsService, vbox );
    page->startAnnotate(m_pathName, rev);

    connect( page, SIGNAL(requestAnnotate(const QString)),
             this, SLOT(slotAnnotate(const QString)) );
}

#include "annotatedialog.moc"
