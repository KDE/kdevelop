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

// stl
#include <string>

// svncpp
#include "kdevsvncpp/log_entry.hpp"
#include "kdevsvncpp/pool.hpp"

// subversion api
#include "svn_time.h"


namespace svn
{
  LogChangePathEntry::LogChangePathEntry (
    const char *path_,
    char action_,
    const char *copyFromPath_,
    const svn_revnum_t copyFromRevision_)
   : path (path_), action(action_), 
     copyFromPath (copyFromPath_ != NULL ? copyFromPath_ : ""),
     copyFromRevision (copyFromRevision_)
  {
  }


  LogEntry::LogEntry ()
  {
  }


  LogEntry::LogEntry (
    const svn_revnum_t revision_,
    const char * author_,
    const char * date_,
    const char * message_)
  {
    date = 0;

    if (date_ != 0)
    {
      Pool pool;

      if (svn_time_from_cstring (&date, date_, pool) != 0)
        date = 0;
    }

    revision = revision_;
    author = author_ == 0 ? "" : author_;
    message = message_ == 0 ? "" : message_;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
