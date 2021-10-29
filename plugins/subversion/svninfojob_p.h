/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNINFOJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNINFOJOB_P_H

#include "svninternaljobbase.h"
#include "svninfojob.h"

class SvnInternalInfoJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalInfoJob( SvnJobBase* parent = nullptr );
    void setLocation( const QUrl& );

    QUrl location() const;
Q_SIGNALS:
    void gotInfo( const SvnInfoHolder& );
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QUrl m_location;
};


#endif

