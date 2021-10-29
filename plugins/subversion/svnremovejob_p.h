/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNREMOVEJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNREMOVEJOB_P_H

#include "svninternaljobbase.h"

class SvnInternalRemoveJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalRemoveJob( SvnJobBase* parent = nullptr );
    void setLocations( const QList<QUrl>& );
    void setForce( bool );

    QList<QUrl> locations() const;

    bool force() const;

protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QList<QUrl> m_locations;
    bool m_force = false;
};


#endif

