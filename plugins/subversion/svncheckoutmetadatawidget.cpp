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

#include <QUrl>
#include <kurlrequester.h>

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

KDevelop::VcsLocation SvnCheckoutMetadataWidget::source() const
{
    KDevelop::VcsLocation src;
    src.setRepositoryServer( m_ui->src->url().url() );
    return src;
}

QUrl SvnCheckoutMetadataWidget::destination() const
{
    return m_ui->dest->url();
}

void SvnCheckoutMetadataWidget::setDestinationLocation( const QUrl &url )
{
    m_ui->dest->setUrl( url );
}

KDevelop::IBasicVersionControl::RecursionMode SvnCheckoutMetadataWidget::recursionMode() const
{
    return m_ui->recurse->isChecked() ? KDevelop::IBasicVersionControl::Recursive : KDevelop::IBasicVersionControl::NonRecursive ;
}


