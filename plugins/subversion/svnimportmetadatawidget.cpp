/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnimportmetadatawidget.h"
#include "ui_importmetadatawidget.h"
#include <vcs/vcslocation.h>

SvnImportMetadataWidget::SvnImportMetadataWidget( QWidget *parent )
    : VcsImportMetadataWidget( parent ), m_ui(new Ui::SvnImportMetadataWidget)
{
    m_ui->setupUi( this );
    m_ui->srcEdit->setUrl( KUrl() );
}

SvnImportMetadataWidget::~SvnImportMetadataWidget()
{
    delete m_ui;
}

void SvnImportMetadataWidget::setSourceLocation( const KDevelop::VcsLocation& importdir )
{
    m_ui->srcEdit->setUrl( importdir.localUrl() );
}

KUrl SvnImportMetadataWidget::source() const
{
    return m_ui->srcEdit->url();
}

KDevelop::VcsLocation SvnImportMetadataWidget::destination() const
{
    KDevelop::VcsLocation destloc;
    destloc.setRepositoryServer(m_ui->dest->url().url());
    return destloc;
}

void SvnImportMetadataWidget::setSourceLocationEditable( bool enable )
{
    m_ui->srcEdit->setEnabled( enable );
}

QString SvnImportMetadataWidget::message() const
{
    return m_ui->message->toPlainText();
}

#include "svnimportmetadatawidget.moc"
