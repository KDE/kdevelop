/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsdiffwidget.h"

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include "../vcsjob.h"
#include "../vcsrevision.h"
#include "../vcsdiff.h"
#include "debug.h"

#include "ui_vcsdiffwidget.h"
#include "vcsdiffpatchsources.h"

namespace KDevelop
{

class VcsDiffWidgetPrivate
{
public:
    Ui::VcsDiffWidget* m_ui;
    VcsJob* m_job;
    VcsDiffWidget* q;

    explicit VcsDiffWidgetPrivate(VcsDiffWidget* _q) : q(_q) {
    }

    void diffReady( KDevelop::VcsJob* job )
    {
        if( job != m_job )
            return;
        KDevelop::VcsDiff diff = m_job->fetchResults().value<KDevelop::VcsDiff>();

        // Try using the patch-review plugin if possible
        auto* patch = new VCSDiffPatchSource(diff);

        if(showVcsDiff(patch))
        {
            q->deleteLater();
            return;
        }else{
            delete patch;
        }

        qCDebug(VCS) << "diff:" << diff.diff();
        m_ui->diffDisplay->setPlainText( diff.diff() );
        m_ui->diffDisplay->setReadOnly( true );

    }

};

VcsDiffWidget::VcsDiffWidget( KDevelop::VcsJob* job, QWidget* parent )
    : QWidget(parent)
    , d_ptr(new VcsDiffWidgetPrivate(this))
{
    Q_D(VcsDiffWidget);

    d->m_job = job;
    d->m_ui = new Ui::VcsDiffWidget();
    d->m_ui->setupUi( this );
    connect( d->m_job, &VcsJob::resultsReady,
             this, [this] (VcsJob* job) { Q_D(VcsDiffWidget); d->diffReady(job); } );
    ICore::self()->runController()->registerJob( d->m_job );
}

VcsDiffWidget::~VcsDiffWidget()
{
    Q_D(VcsDiffWidget);

    delete d->m_ui;
}

void VcsDiffWidget::setRevisions( const KDevelop::VcsRevision& first,
                                  const KDevelop::VcsRevision& second )
{
    Q_D(VcsDiffWidget);

    d->m_ui->revLabel->setText( i18nc("@label", "Difference between revision %1 and %2:",
                       first.prettyValue(),
                       second.prettyValue() ) );
}

}

#include "moc_vcsdiffwidget.cpp"
