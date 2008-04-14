/*
 * ====================================================================
 * Copyright (c) 2002-2008 The RapidSvn Group.  All rights reserved.
 * Copyright (c) 2008 Andreas Pakulat <apaku@gmx.de>
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

// svncpp
#include "kdevsvncpp/info.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/pool.hpp"

#include <cassert>

namespace svn
{
  struct Info::Data
  {
    svn_info_t * info;
    Path path;
    Pool pool;
  
    /** constructor (because of optional param */
    Data (const Path & path_, const svn_info_t * info_ = 0)
      : info (0), path (path_)
    {
      if (info_ != 0)
        info = svn_info_dup (info_, pool);
    }
  
    /** copy constructor */
    Data (const Data * src)
      : info (0), path (src->path)
    {
      if (src->info != 0)
        info = svn_info_dup (src->info, pool);
    }
  };
    
  Info::Info (const Path & path, const svn_info_t * info)
    : m (new Data (path, info))
  {
  }

  Info::Info (const Info & src)
    : m (new Data (src.m))
  {
  }

  Info::~Info ()
  {
    delete m;
  }


  Info &
  Info::operator = (const Info & src)
  {
    if (this != &src)
    {
      delete m;
      m = new Data (src.m);
    }

    return *this;
  }

  const svn_node_kind_t
  Info::
  kind () const
  {
    if (isValid())
      return svn_node_none;
    else
      return m->info->kind;
  }

  bool 
  Info::isValid () const
  {
    return m->info != 0;
  }


  const char * 
  Info::url () const
  {
    if (isValid())
      return 0;
    else
      return m->info->URL;
  }

  const Path & 
  Info::path () const
  {
    return m->path;
  }
  
  const svn_revnum_t 
  Info::revision () const
  {
    if (isValid())
      return 0;
    else
      return m->info->rev;
  }

  const char *
  Info::repos () const
  {
    if (isValid())
      return 0;
    else
      return m->info->repos_root_URL;
  }
  
  const char *
  Info::uuid () const {
    if (isValid())
      return 0;
    else
      return m->info->repos_UUID;
  }
  
  svn_revnum_t
  Info::lastChangedRevision () const
  {
    if (isValid())
      return 0;
    else
      return m->info->last_changed_rev;
  }

  apr_time_t
  Info::lastChangedDate () const
  {
    if (isValid())
      return 0;
    else
      return m->info->last_changed_date;
  }

  const char *
  Info::lastChangedAuthor () const
  {
    if (isValid())
      return 0;
    else
      return m->info->last_changed_author;
  }
  
  bool
  Info::hasWcInfo () const 
  {
    if (isValid())
      return 0;
    else
      return m->info->has_wc_info;
  }
  
  svn_wc_schedule_t Info::schedule () const
  {
    assert(m->info);
    return m->info->schedule;
  }
  
  const char* Info::copyFromUrl () const
  {
    if (isValid())
      return 0;
    else
      return m->info->copyfrom_url;
  }
  
  svn_revnum_t Info::copyFromRevision () const
  {
    if (isValid())
      return 0;
    else
      return m->info->copyfrom_rev;
  }
  
  apr_time_t Info::textTime () const
  {
    if (isValid())
      return 0;
    else
      return m->info->text_time;
  }
  
  apr_time_t Info::propertyTime () const
  {
    if (isValid())
      return 0;
    else
      return m->info->prop_time;
  }
  
  const char* Info::oldConflictFile () const
  {
    if (isValid())
      return 0;
    else
      return m->info->conflict_old;
  }

  const char* Info::newConflictFile () const
  {
    if (isValid())
      return 0;
    else
      return m->info->conflict_new;
  }

  const char* Info::workingConflictFile () const
  {
    if (isValid())
      return 0;
    else
      return m->info->conflict_wrk;
  }
  
  const char* Info::propertyRejectFile () const 
  {
    if (isValid())
      return 0;
    else
      return m->info->prejfile;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
