/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsjob.h"

//#include <KLocalizedString>

namespace KDevelop {

class VcsJobPrivate
{
public:
    VcsJob::JobType m_type;
};

VcsJob::VcsJob( QObject* parent, OutputJobVerbosity verbosity )
    : OutputJob(parent, verbosity)
    , d_ptr(new VcsJobPrivate)
{
    Q_D(VcsJob);

    d->m_type = Unknown;
    setStandardToolView(IOutputView::VcsView);

    if(verbosity == Verbose) {
        QMetaObject::invokeMethod(this, "delayedModelInitialize", Qt::QueuedConnection);
    }
}

void VcsJob::delayedModelInitialize()
{
    startOutput();
}

VcsJob::~VcsJob() = default;

VcsJob::JobType VcsJob::type() const
{
    Q_D(const VcsJob);

    return d->m_type;
}

void VcsJob::setType( VcsJob::JobType t )
{
    Q_D(VcsJob);

    d->m_type = t;
}

}

#include "moc_vcsjob.cpp"
