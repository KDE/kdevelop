/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNREVERTJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNREVERTJOB_P_H

#include "svninternaljobbase.h"

class SvnInternalRevertJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalRevertJob( SvnJobBase* parent = nullptr );
    void setLocations( const QList<QUrl>& );
    void setRecursive( bool );

    bool recursive() const;
    QList<QUrl> locations() const;
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QList<QUrl> m_locations;
    bool m_recursive = false;
};

#endif

