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

#include "svn_copywidgets.h"
#include "svn_revisionwidget.h"
#include <kurl.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <QCheckBox>
#include <QRadioButton>

SvnCopyOptionDlg::SvnCopyOptionDlg( const KUrl &reqUrl, SvnInfoHolder *info,
                                    QWidget *parent )
    : KDialog( parent ), m_reqUrl(reqUrl), m_info(info)
{
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( "Subversion Copy" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    ui.urlRadio->setChecked(true);

    ui.revisionWidget->setKey( SvnRevision::HEAD );
    ui.revisionWidget->enableType( SvnRevision::kind );

    ui.reqEdit->setText( reqUrl.prettyUrl() );

    ui.destEdit->setMode( KFile::File | KFile::Directory );

    srcAsUrlClicked();

    connect( ui.urlRadio, SIGNAL(clicked()), this, SLOT(srcAsUrlClicked()) );
    connect( ui.pathRadio, SIGNAL(clicked()), this, SLOT(srcAsPathClicked()) );
}

SvnCopyOptionDlg::~SvnCopyOptionDlg()
{}

KUrl SvnCopyOptionDlg::source()
{
    return KUrl( ui.srcEdit->text() );
}

SvnRevision SvnCopyOptionDlg::sourceRev()
{
    return ui.revisionWidget->revision();
}

KUrl SvnCopyOptionDlg::dest()
{
    return ui.destEdit->url();
}

void SvnCopyOptionDlg::srcAsUrlClicked()
{
    ui.srcEdit->setText( m_info->URL );

    KUrl srcUrl( m_info->URL );
    KUrl destParent = srcUrl.upUrl();
    ui.destEdit->setUrl( destParent );

    QList<SvnRevision::RevKeyword> keylist;
    keylist << SvnRevision::HEAD;
    ui.revisionWidget->installKeys( keylist );
}

void SvnCopyOptionDlg::srcAsPathClicked()
{
    ui.srcEdit->setText( m_reqUrl.toLocalFile() );

    KUrl srcUrl( m_reqUrl.toLocalFile() );
    KUrl destParent = srcUrl.upUrl();
    ui.destEdit->setPath( destParent.toLocalFile() );

    QList<SvnRevision::RevKeyword> keylist;
    keylist << SvnRevision::WORKING;
    ui.revisionWidget->installKeys( keylist );
}

#include "svn_copywidgets.moc"

