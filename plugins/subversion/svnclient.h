/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCLIENT_H
#define KDEVPLATFORM_PLUGIN_SVNCLIENT_H

#include <QObject>

#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/revision.hpp"
#include "kdevsvncpp/client.hpp"

#include <vcs/vcsevent.h>

class SvnClient : public QObject, public svn::Client
{
    Q_OBJECT
public:
    explicit SvnClient( svn::Context* = nullptr );

    QString diff( const svn::Path& src, const svn::Revision& srcRev,
                  const svn::Path& dst, const svn::Revision& dstRev,
                  const bool recurse, const bool ignoreAncestry,
                  const bool noDiffDeleted, const bool ignoreContentType );

    QString diff( const svn::Path& src, const svn::Revision& pegRev,
                  const svn::Revision& srcRev, const svn::Revision& dstRev,
                  const bool recurse, const bool ignoreAncestry,
                  const bool noDiffDeleted, const bool ignoreContentType );

    void log( const char* path,
              const svn::Revision& start,
              const svn::Revision& end,
              int limit,
              bool discoverChangedPaths = false,
              bool strictNodeHistory = true );

    void emitLogEventReceived( const KDevelop::VcsEvent& );

Q_SIGNALS:
    void logEventReceived( const KDevelop::VcsEvent& );
private:
    svn::Context* m_ctxt;
};

#endif

