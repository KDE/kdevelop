/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "svndiffwidget.h"

#include <QLabel>

#include <ktextedit.h>
#include <kdebug.h>

#include <vcsjob.h>
#include <vcsrevision.h>
#include <vcsdiff.h>

SvnDiffWidget::SvnDiffWidget( KDevelop::VcsJob* job, QWidget* parent )
    : QWidget( parent ), m_job( job )
{
    setupUi( this );
    connect( m_job, SIGNAL( resultsReady( KDevelop::VcsJob *) ),
             this, SLOT( diffReady( KDevelop::VcsJob* ) ) );
    m_job->start();
}

void SvnDiffWidget::setRevisions( const KDevelop::VcsRevision& first,
                                  const KDevelop::VcsRevision& second )
{
    revLabel->setText( i18n("Difference between revision %1 and %2:",
                       first.prettyValue(),
                       second.prettyValue() ) );
}

void SvnDiffWidget::diffReady( KDevelop::VcsJob* job )
{
    if( job != m_job )
        return;
    KDevelop::VcsDiff diff = qVariantValue<KDevelop::VcsDiff>( m_job->fetchResults() );

    kDebug(9510) << "diff:" << diff.leftTexts().count();
    foreach( KDevelop::VcsLocation l, diff.leftTexts().keys() )
    {
        kDebug(9510) << "diff:" << l.localUrl() << l.repositoryServer();
    }
    kDebug(9510) << "diff:" << diff.diff();
    kDebug(9510) << "diff:" << diff.type();
    kDebug(9510) << "diff:" << diff.contentType();
    diffDisplay->setPlainText( diff.diff() );
    diffDisplay->setReadOnly( true );

}


SvnDiffDialog::SvnDiffDialog( KDevelop::VcsJob* job, QWidget* parent )
    : KDialog( parent ), m_widget( new SvnDiffWidget(job, this) )
{
    setButtons( KDialog::Close );
    setMainWidget( m_widget );
}

void SvnDiffDialog::setRevisions( const KDevelop::VcsRevision& rev1,
                                  const KDevelop::VcsRevision& rev2 )
{
    m_widget->setRevisions( rev1, rev2 );
}

