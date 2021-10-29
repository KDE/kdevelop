/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNBLAMEJOB_H
#define KDEVPLATFORM_PLUGIN_SVNBLAMEJOB_H


#include "svnjobbase.h"

#include <QVariant>

#include <QUrl>

#include <vcs/vcsrevision.h>
#include <vcs/vcsannotation.h>

class SvnInternalBlameJob;

class SvnBlameJob : public SvnJobBaseImpl<SvnInternalBlameJob>
{
    Q_OBJECT
public:
    explicit SvnBlameJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setLocation( const QUrl &location );
    void setEndRevision( const KDevelop::VcsRevision& rev );
    void setStartRevision( const KDevelop::VcsRevision& rev );
private Q_SLOTS:
    void blameLineReceived( const KDevelop::VcsAnnotationLine& );

private:
    QList<QVariant> m_annotations;

};


#endif
