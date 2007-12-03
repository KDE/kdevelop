/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svncheckoutmetadatawidget.h"
#include "ui_checkoutmetadatawidget.h"

#include <kurl.h>
#include <kurlrequester.h>

#include <vcs/vcsmapping.h>
#include <vcs/vcslocation.h>

SvnCheckoutMetadataWidget::SvnCheckoutMetadataWidget( QWidget *parent )
    : QWidget( parent ), m_ui( new Ui::SvnCheckoutMetadataWidget )
{
    m_ui->setupUi( this );
}

SvnCheckoutMetadataWidget::~SvnCheckoutMetadataWidget()
{
    delete m_ui;
}

KDevelop::VcsMapping SvnCheckoutMetadataWidget::mapping() const
{
    KDevelop::VcsMapping map;
    KDevelop::VcsLocation src;
    src.setRepositoryServer( m_ui->src->url().url() );
    map.addMapping( src, KDevelop::VcsLocation( m_ui->dest->url() ), m_ui->recurse->isChecked() ? KDevelop::VcsMapping::Recursive : KDevelop::VcsMapping::NonRecursive );
    return map;
}

void SvnCheckoutMetadataWidget::setDestinationLocation( const KUrl& url )
{
    m_ui->dest->setUrl( url );
}

#include "svncheckoutmetadatawidget.moc"

