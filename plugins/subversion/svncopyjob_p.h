/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCOPYJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNCOPYJOB_P_H

#include "svninternaljobbase.h"

class SvnInternalCopyJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalCopyJob( SvnJobBase* parent = nullptr );
    void setSourceLocation( const QUrl& );
    void setDestinationLocation( const QUrl& );

    QUrl sourceLocation() const;
    QUrl destinationLocation() const;
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QUrl m_sourceLocation;
    QUrl m_destinationLocation;
};


#endif
