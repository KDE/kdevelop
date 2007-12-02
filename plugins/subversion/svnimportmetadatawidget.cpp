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
#include <vcsmapping.h>
#include <vcslocation.h>

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

KDevelop::VcsMapping SvnImportMetadataWidget::mapping() const
{
    KDevelop::VcsLocation loc;
    loc.setLocalUrl( m_ui->srcEdit->url() );
    KDevelop::VcsLocation destloc;
    destloc.setRepositoryServer(m_ui->dest->url().url());
    KDevelop::VcsMapping map;
    map.addMapping( loc, destloc , m_ui->recursive->isChecked() ? KDevelop::VcsMapping::Recursive : KDevelop::VcsMapping::NonRecursive );
    return map;
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
