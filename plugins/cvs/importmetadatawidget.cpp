/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "importmetadatawidget.h"

#include <KMessageBox>
#include <KDebug>

#include <vcs/vcslocation.h>

ImportMetadataWidget::ImportMetadataWidget(QWidget *parent)
    : KDevelop::VcsImportMetadataWidget(parent), m_ui( new Ui::ImportMetadataWidget )
{
    m_ui->setupUi(this);

    m_ui->sourceLoc->setEnabled( false );
    m_ui->sourceLoc->setMode( KFile::Directory );

    connect( m_ui->sourceLoc, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
    connect( m_ui->sourceLoc, SIGNAL(urlSelected(KUrl)), SIGNAL(changed()) );
    connect( m_ui->comment, SIGNAL(textChanged()), SIGNAL(changed()) );
    connect( m_ui->module, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( m_ui->releaseTag, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( m_ui->repository, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    connect( m_ui->vendorTag, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
    
}

ImportMetadataWidget::~ImportMetadataWidget()
{
    delete m_ui;
}

KUrl ImportMetadataWidget::source() const
{
    return m_ui->sourceLoc->url() ;
}

KDevelop::VcsLocation ImportMetadataWidget::destination() const
{
    KDevelop::VcsLocation destloc;
    destloc.setRepositoryServer(m_ui->repository->text() );
    destloc.setRepositoryModule(m_ui->module->text());
    destloc.setRepositoryTag(m_ui->vendorTag->text());
    destloc.setUserData(m_ui->releaseTag->text());
    return destloc;
}

QString ImportMetadataWidget::message( ) const
{
    return m_ui->comment->toPlainText();
}

void ImportMetadataWidget::setSourceLocation( const KDevelop::VcsLocation& url )
{
    m_ui->sourceLoc->setUrl( url.localUrl() );
}

void ImportMetadataWidget::setSourceLocationEditable( bool enable )
{
    m_ui->sourceLoc->setEnabled( enable );
}

bool ImportMetadataWidget::hasValidData() const
{
    return !m_ui->comment->toPlainText().isEmpty() && !m_ui->sourceLoc->text().isEmpty()
    && !m_ui->module->text().isEmpty() && !m_ui->repository->text().isEmpty();
}

#include "importmetadatawidget.moc"
