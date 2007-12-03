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

#include <vcs/vcsmapping.h>
#include <vcs/vcslocation.h>

ImportMetadataWidget::ImportMetadataWidget(QWidget *parent)
    : KDevelop::VcsImportMetadataWidget(parent), m_ui( new Ui::ImportMetadataWidget )
{
    m_ui->setupUi(this);

    m_ui->sourceLoc->setEnabled( false );
    m_ui->sourceLoc->setMode( KFile::Directory );
}

ImportMetadataWidget::~ImportMetadataWidget()
{
    delete m_ui;
}

KDevelop::VcsMapping ImportMetadataWidget::mapping( ) const
{
    KDevelop::VcsLocation loc;
    loc.setLocalUrl( m_ui->sourceLoc->url() );
    KDevelop::VcsLocation destloc;
    destloc.setRepositoryServer(m_ui->repository->text() );
    destloc.setRepositoryModule(m_ui->module->text());
    destloc.setRepositoryTag(m_ui->vendorTag->text());
    destloc.setRepositoryBranch(m_ui->releaseTag->text());
    KDevelop::VcsMapping map;
    map.addMapping( loc, destloc , KDevelop::VcsMapping::Recursive );
    return map;
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

#include "importmetadatawidget.moc"
