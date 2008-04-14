/*
 * ====================================================================
 * Copyright (c) 2002-2008 The RapidSvn Group.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library (in the file LGPL.txt); if not, 
 * write to the Free Software Foundation, Inc., 51 Franklin St, 
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef _SVNCPP_LOG_ENTRY_H_
#define _SVNCPP_LOG_ENTRY_H_

// stl
#include <string>
#include <list>

// apr
#include "apr_time.h"

// subversion api
#include "svn_types.h"

namespace svn
{

  struct LogChangePathEntry
  {
    LogChangePathEntry (const char *path_,
                        char action_,
                        const char *copyFromPath_,
                        const svn_revnum_t copyFromRevision_);

    std::string path;
    char action;
    std::string copyFromPath;
    svn_revnum_t copyFromRevision;
  };


  struct LogEntry
  {
  public:
    LogEntry ();

    LogEntry (const svn_revnum_t revision,
              const char * author,
              const char * date,
              const char * message);

    svn_revnum_t revision;
    std::string author;
    std::string message;
    std::list<LogChangePathEntry> changedPaths;
    apr_time_t date;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */

