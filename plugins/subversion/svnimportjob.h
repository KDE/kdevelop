/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNIMPORTJOB_H
#define KDEVPLATFORM_PLUGIN_SVNIMPORTJOB_H

#include "svnjobbase.h"

class SvnImportInternalJob;

class SvnImportJob : public SvnJobBaseImpl<SvnImportInternalJob>
{
    Q_OBJECT
public:
    explicit SvnImportJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setMapping( const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository);
    void setMessage( const QString& );
};


#endif

