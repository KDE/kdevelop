/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svncatwidgets.h"
#include "svnrevisionwidget.h"
#include <kurlrequester.h>
#include <QList>

SvnCatOptionDlg::SvnCatOptionDlg( const KUrl &path, QWidget *parent )
    : KDialog( parent )
{
    ui.setupUi(mainWidget());
    setCaption( "Subversion Cat" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    QList<SvnRevision::Keyword> keys;
    keys << SvnRevision::Head << SvnRevision::Base << SvnRevision::Committed << SvnRevision::Prev;
    ui.revisionWidget->installKeys(keys);
    ui.revisionWidget->setKey( SvnRevision::Head );
    ui.revisionWidget->enableType( SvnRevision::Kind );

    ui.urledit->setUrl( path );
}

SvnCatOptionDlg::~SvnCatOptionDlg()
{}

KUrl SvnCatOptionDlg::url()
{
    return ui.urledit->url();
}

SvnRevision SvnCatOptionDlg::revision()
{
    return ui.revisionWidget->revision();
}

#include "svncatwidgets.moc"
