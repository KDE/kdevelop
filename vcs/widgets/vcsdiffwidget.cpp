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

#include "vcsdiffwidget.h"

#include <QLabel>

#include <ktextedit.h>
#include <kdebug.h>

#include "../vcsjob.h"
#include "../vcsrevision.h"
#include "../vcsdiff.h"

#include "ui_vcsdiffwidget.h"

namespace KDevelop
{

class VcsDiffWidgetPrivate
{
public:
    Ui::VcsDiffWidget* m_ui;
    VcsJob* m_job;
    void diffReady( KDevelop::VcsJob* job )
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
        m_ui->diffDisplay->setPlainText( diff.diff() );
        m_ui->diffDisplay->setReadOnly( true );
    
    }

};
    
VcsDiffWidget::VcsDiffWidget( KDevelop::VcsJob* job, QWidget* parent )
    : QWidget( parent ), d(new VcsDiffWidgetPrivate)
{
    d->m_job = job;
    d->m_ui = new Ui::VcsDiffWidget();
    d->m_ui->setupUi( this );
    connect( d->m_job, SIGNAL( resultsReady( KDevelop::VcsJob *) ),
             this, SLOT( diffReady( KDevelop::VcsJob* ) ) );
    d->m_job->start();
}

void VcsDiffWidget::setRevisions( const KDevelop::VcsRevision& first,
                                  const KDevelop::VcsRevision& second )
{
    d->m_ui->revLabel->setText( i18n("Difference between revision %1 and %2:",
                       first.prettyValue(),
                       second.prettyValue() ) );
}

}

#include "vcsdiffwidget.moc"
