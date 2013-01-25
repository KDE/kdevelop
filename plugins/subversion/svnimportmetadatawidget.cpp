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
    , useSourceDirForDestination( false )
{
    m_ui->setupUi( this );
    m_ui->srcEdit->setUrl( KUrl() );
    connect( m_ui->srcEdit, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
    connect( m_ui->srcEdit, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( m_ui->dest, SIGNAL(textChanged(QString)), this, SIGNAL(changed()) );
    connect( m_ui->message, SIGNAL(textChanged()), this, SIGNAL(changed()) );
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
    QString url = m_ui->dest->text();
    if( useSourceDirForDestination ) {
        url += '/' + m_ui->srcEdit->url().fileName();
    }
    destloc.setRepositoryServer(url);
    return destloc;
}

void SvnImportMetadataWidget::setUseSourceDirForDestination( bool b )
{
    useSourceDirForDestination = b;
}


void SvnImportMetadataWidget::setSourceLocationEditable( bool enable )
{
    m_ui->srcEdit->setEnabled( enable );
}

QString SvnImportMetadataWidget::message() const
{
    return m_ui->message->toPlainText();
}

bool SvnImportMetadataWidget::hasValidData() const
{
    return !m_ui->message->toPlainText().isEmpty() && !m_ui->srcEdit->text().isEmpty();
}


#include "svnimportmetadatawidget.moc"
