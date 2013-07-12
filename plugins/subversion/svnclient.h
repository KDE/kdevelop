/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVNCLIENT_H
#define KDEVPLATFORM_PLUGIN_SVNCLIENT_H

#include <QString>
#include <QObject>

#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/revision.hpp"
#include "kdevsvncpp/client.hpp"

#include <vcs/vcsevent.h>
namespace KDevelop
{
class VcsAnnotationLine;
}
class SvnClient : public QObject, public svn::Client
{
    Q_OBJECT
public:
    SvnClient( svn::Context* = 0 );

    QString diff( const svn::Path& src, const svn::Revision& srcRev,
                  const svn::Path& dst, const svn::Revision& dstRev,
                  const bool recurse, const bool ignoreAncestry,
                  const bool noDiffDeleted, const bool ignoreContentType )
            throw (svn::ClientException);

    QString diff( const svn::Path& src, const svn::Revision& pegRev,
                  const svn::Revision& srcRev, const svn::Revision& dstRev,
                  const bool recurse, const bool ignoreAncestry,
                  const bool noDiffDeleted, const bool ignoreContentType )
            throw (svn::ClientException);

    void log( const char* path,
              const svn::Revision& start,
              const svn::Revision& end,
              int limit,
              bool discoverChangedPaths = false,
              bool strictNodeHistory = true )
            throw (svn::ClientException);

    void emitLogEventReceived( const KDevelop::VcsEvent& );

signals:
    void logEventReceived( const KDevelop::VcsEvent& );
private:
    svn::Context* m_ctxt;
};

#endif

