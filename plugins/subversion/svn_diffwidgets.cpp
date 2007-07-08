/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svn_diffwidgets.h"
#include "svn_revisionwidget.h"
#include <kurl.h>

SvnPegDiffDialog::SvnPegDiffDialog( QWidget *parent )
    : KDialog(parent)
{
    QWidget *widget = new QWidget( this );
    ui.setupUi( widget );

    // KDialog specific
    setMainWidget( widget );
    setCaption( "Subversion Diff" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    // setup revision widgets
    m_startRev = new SvnRevisionWidget( ui.startRevWidget );
    QVBoxLayout *startL = new QVBoxLayout( ui.startRevWidget );
    startL->addWidget( m_startRev );

    m_endRev = new SvnRevisionWidget( ui.endRevWidget );
    QVBoxLayout *endL = new QVBoxLayout( ui.endRevWidget );
    endL->addWidget( m_endRev );

    // set default revision
    m_startRev->setNumber(1);
    m_startRev->enableType( SvnRevision::number );
    m_endRev->setKey( SvnRevision::WORKING );
    m_endRev->enableType( SvnRevision::kind );
}

SvnPegDiffDialog::~SvnPegDiffDialog()
{
}

void SvnPegDiffDialog::setUrl( const KUrl& url )
{
    ui.urlEdit->setUrl( url );
}

SvnRevision SvnPegDiffDialog::startRev()
{
    return m_startRev->revision();
}

SvnRevision SvnPegDiffDialog::endRev()
{
    return m_endRev->revision();
}

bool SvnPegDiffDialog::recurse()
{
    return !(ui.nonrecurseChk->isChecked());
}

bool SvnPegDiffDialog::noDiffDeleted()
{
    return ui.nodiffDeletedChk->isChecked();
}

bool SvnPegDiffDialog::ignoreContentType()
{
    return ui.ignoreContentsChk->isChecked();
}

#include "svn_diffwidgets.moc"


