/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNIMPORTJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNIMPORTJOB_P_H

#include "svninternaljobbase.h"

class SvnImportInternalJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnImportInternalJob( SvnJobBase* parent = nullptr );
    void setMapping( const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository);
    void setMessage( const QString& );

    QString message() const;
    bool isValid() const;
    QUrl source() const;
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QUrl m_sourceDirectory;
    KDevelop::VcsLocation m_destinationRepository;
    QString m_message;
};

#endif
