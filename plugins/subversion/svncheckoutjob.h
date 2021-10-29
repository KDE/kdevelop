/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCHECKOUTJOB_H
#define KDEVPLATFORM_PLUGIN_SVNCHECKOUTJOB_H

#include "svnjobbase.h"
#include <vcs/interfaces/ibasicversioncontrol.h>

namespace KDevelop
{
class VcsLocation;
}

class SvnInternalCheckoutJob;

class SvnCheckoutJob : public SvnJobBaseImpl<SvnInternalCheckoutJob>
{
    Q_OBJECT
public:
    explicit SvnCheckoutJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setMapping( const KDevelop::VcsLocation & sourceRepository, const QUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);
};



#endif

