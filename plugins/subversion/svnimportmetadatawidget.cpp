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

SvnImportMetadataWidget::SvnImportMetadataWidget( QWidget *parent )
    : VcsImportMetadataWidget( parent ), m_ui(new Ui::SvnImportMetadataWidget)
{
    m_ui->setupUi( this );
    m_ui->srcLabel->setText( "" );
}

SvnImportMetadataWidget::~SvnImportMetadataWidget()
{
    delete m_ui;
}

void SvnImportMetadataWidget::setImportDirectory( const QString& importdir )
{
    m_ui->srcLabel->setText( importdir );
}

KDevelop::VcsMapping SvnImportMetadataWidget::mapping() const
{
    QString importdir = m_ui->srcLabel->text();
    KDevelop::VcsMapping map;
    map.addMapping( importdir, m_ui->dest->url().url(), m_ui->recursive->isChecked() ? KDevelop::VcsMapping::Recursive : KDevelop::VcsMapping::NonRecursive );
    return map;
}

#include "svnimportmetadatawidget.moc"
