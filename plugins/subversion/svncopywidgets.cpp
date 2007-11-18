/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svncopywidgets.h"
#include "svnrevisionwidget.h"
#include <kurl.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <QCheckBox>
#include <QRadioButton>

SvnCopyOptionDlg::SvnCopyOptionDlg( const KUrl &reqUrl, SvnInfoHolder *info,
                                    QWidget *parent )
    : KDialog( parent ), m_reqUrl(reqUrl), m_info(info)
{
    ui.setupUi(mainWidget());
    setCaption( "Subversion Copy" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    ui.urlRadio->setChecked(true);

    ui.revisionWidget->setKey( SvnRevision::Head );
    ui.revisionWidget->enableType( SvnRevision::Kind );

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
    if( m_info ){
        ui.srcEdit->setUrl( m_info->url );
        KUrl srcUrl( m_info->url );
        ui.destEdit->setUrl( srcUrl.upUrl() );
    }

    QList<SvnRevision::Keyword> keylist;
    keylist << SvnRevision::Head;
    ui.revisionWidget->installKeys( keylist );
}

void SvnCopyOptionDlg::srcAsPathClicked()
{
    ui.srcEdit->setText( m_reqUrl.toLocalFile() );

    KUrl srcUrl( m_reqUrl.toLocalFile() );
    KUrl destParent = srcUrl.upUrl();
    ui.destEdit->setPath( destParent.toLocalFile() );

    QList<SvnRevision::Keyword> keylist;
    keylist << SvnRevision::Working;
    ui.revisionWidget->installKeys( keylist );
}

#include "svncopywidgets.moc"

