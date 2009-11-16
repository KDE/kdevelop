/*
 * ====================================================================
 * Copyright (c) 2002-2009 The RapidSvn Group.  All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the file GPL.txt.  
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef _SVNCPP_LOG_ENTRY_H_
#define _SVNCPP_LOG_ENTRY_H_

// stl
#include "svncpp/string_wrapper.hpp"
#include <list>

// apr
#include "apr_time.h"

// subversion api
#include "svn_types.h"

namespace svn
{

  struct LogChangePathEntry
  {
    LogChangePathEntry(const char *path_,
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
    LogEntry();

    LogEntry(const svn_revnum_t revision,
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

