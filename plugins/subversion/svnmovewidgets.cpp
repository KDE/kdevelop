/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnmovewidgets.h"
#include <QUrl>
#include <kurlrequester.h>
#include <klineedit.h>
#include <QCheckBox>
#include <QRadioButton>

SvnMoveOptionDlg::SvnMoveOptionDlg( const QUrl &reqUrl, SvnInfoHolder *info, QWidget *parent )
    : KDialog( parent ), m_reqUrl(reqUrl), m_info(info)
{
    ui.setupUi(mainWidget());
    setCaption( i18n("Subversion Move" ));
    setButtons( KDialog::Ok | KDialog::Cancel );

    ui.urlRadio->setChecked(true);
    ui.reqEdit->setText( reqUrl.toDisplayString(QUrl::PreferLocalFile) );
    ui.destEdit->setMode( KFile::File | KFile::Directory );
    srcAsUrlClicked();

    connect( ui.urlRadio, SIGNAL(clicked()), this, SLOT(srcAsUrlClicked()) );
    connect( ui.pathRadio, SIGNAL(clicked()), this, SLOT(srcAsPathClicked()) );
}

SvnMoveOptionDlg::~SvnMoveOptionDlg()
{}

QUrl SvnMoveOptionDlg::source()
{
    return QUrl::fromUserInput( ui.srcEdit->text() );
}

QUrl SvnMoveOptionDlg::dest()
{
    return ui.destEdit->url();
}

bool SvnMoveOptionDlg::force()
{
    return ui.forceChk->isChecked();
}

void SvnMoveOptionDlg::srcAsUrlClicked()
{
    if( m_info ){
        ui.srcEdit->setUrl( m_info->url );
        QUrl srcUrl( m_info->url );
        ui.destEdit->setUrl( srcUrl.resolved(QStringLiteral("..")) );
    }
}

void SvnMoveOptionDlg::srcAsPathClicked()
{
    ui.srcEdit->setText( m_reqUrl.toLocalFile() );

    QUrl srcUrl = QUrl::fromLocalFile( m_reqUrl.toLocalFile() );
    QUrl destParent = srcUrl.resolved( QStringLiteral("..") );
    ui.destEdit->setPath( destParent.toLocalFile() );
}


#include "svnmovewidgets.moc"
