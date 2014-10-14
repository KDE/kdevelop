/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Pimpl-ed and exported                                                 *
 *   Copyright 2014 Maciej Poleski                                         *
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

#include "dvcsimportmetadatawidget.h"

#include <KMessageBox>

#include <vcslocation.h>
#include "ui_dvcsimportmetadatawidget.h"

class DvcsImportMetadataWidgetPrivate
{
    friend class DvcsImportMetadataWidget;

    DvcsImportMetadataWidgetPrivate(Ui::DvcsImportMetadataWidget* ui) : m_ui(ui) {}
    ~DvcsImportMetadataWidgetPrivate() { delete m_ui; }

    Ui::DvcsImportMetadataWidget* m_ui;
};

DvcsImportMetadataWidget::DvcsImportMetadataWidget(QWidget *parent)
    : KDevelop::VcsImportMetadataWidget(parent),
    d_ptr(new DvcsImportMetadataWidgetPrivate(new Ui::DvcsImportMetadataWidget))
{
    Q_D(DvcsImportMetadataWidget);
    d->m_ui->setupUi(this);

    d->m_ui->sourceLoc->setEnabled( false );
    d->m_ui->sourceLoc->setMode( KFile::Directory );
    connect( d->m_ui->sourceLoc, SIGNAL(textChanged(QString)), this, SIGNAL(changed()) );
    connect( d->m_ui->sourceLoc, SIGNAL(urlSelected(QUrl)), this, SIGNAL(changed()) );
}

DvcsImportMetadataWidget::~DvcsImportMetadataWidget()
{
    delete d_ptr;
}

QUrl DvcsImportMetadataWidget::source() const
{
    Q_D(const DvcsImportMetadataWidget);
    return d->m_ui->sourceLoc->url();
}

KDevelop::VcsLocation DvcsImportMetadataWidget::destination() const
{
    // Used for compatibility with import
    Q_D(const DvcsImportMetadataWidget);
    KDevelop::VcsLocation dest;
    dest.setRepositoryServer(d->m_ui->sourceLoc->url().url());
    return dest;
}

QString DvcsImportMetadataWidget::message( ) const
{
    return QString();
}

void DvcsImportMetadataWidget::setSourceLocation( const KDevelop::VcsLocation& url )
{
    Q_D(const DvcsImportMetadataWidget);
    d->m_ui->sourceLoc->setUrl( url.localUrl() );
}

void DvcsImportMetadataWidget::setSourceLocationEditable( bool enable )
{
    Q_D(const DvcsImportMetadataWidget);
    d->m_ui->sourceLoc->setEnabled( enable );
}

bool DvcsImportMetadataWidget::hasValidData() const
{
    Q_D(const DvcsImportMetadataWidget);
    return !d->m_ui->sourceLoc->text().isEmpty();
}

