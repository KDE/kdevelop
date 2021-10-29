/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCHECKOUTJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNCHECKOUTJOB_P_H


#include "svninternaljobbase.h"
#include <vcs/interfaces/ibasicversioncontrol.h>

class SvnInternalCheckoutJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalCheckoutJob( SvnJobBase* parent = nullptr );
    void setMapping(const KDevelop::VcsLocation & sourceRepository, const QUrl &destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion);

    bool isValid() const;
    KDevelop::VcsLocation source() const;
    QUrl destination() const;
    KDevelop::IBasicVersionControl::RecursionMode recursion() const;
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    KDevelop::VcsLocation m_sourceRepository;
    QUrl m_destinationDirectory;
    KDevelop::IBasicVersionControl::RecursionMode m_recursion = KDevelop::IBasicVersionControl::Recursive;
};


#endif

