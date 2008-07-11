/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
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

QString ImportMetadataWidget::getRepositoryLocation() const
{
    return m_ui->sourceLoc->url().path();
}

KDevelop::VcsMapping ImportMetadataWidget::mapping( ) const
{
    KDevelop::VcsLocation loc;
    loc.setLocalUrl( m_ui->sourceLoc->url() );
    KDevelop::VcsLocation dest = loc; // Used for compatibility with VcsMapping
    KDevelop::VcsMapping map;
    map.addMapping(loc, dest, KDevelop::VcsMapping::Recursive);
    return map;
}

QString ImportMetadataWidget::message( ) const
{
    return QString("");
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
