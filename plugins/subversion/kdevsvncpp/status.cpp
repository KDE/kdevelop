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
#include "kdevsvncpp/status.hpp"

namespace svn
{
  struct Status::Data
  {
    svn_wc_status2_t *status;
    std::string path;
    Pool pool;
    bool isVersioned;

    Data (const char * path_, const svn_wc_status2_t * status_)
      : status (0), path ("") 
    {
      if (path_ != 0)
        path = path_;

      if (status_ != 0)
      {
        status = svn_wc_dup_status2 (
          const_cast<svn_wc_status2_t *> (status_), pool);
        isVersioned = status_->text_status > svn_wc_status_unversioned;
      }
    }

    Data (const Data * src)
      : status (0), path (src->path)
    {
      if (src->status != 0)
      {
        status = svn_wc_dup_status2 (src->status, pool);
        isVersioned = status->text_status > svn_wc_status_unversioned;
      }
    }

  };

  Status::Status (const char * path, const svn_wc_status2_t * status)
    : m (new Data (path, status))
  {
  }

  Status::Status (const Status & src)
    : m (new Data (src.m))
  {
  }

  Status::~Status ()
  {
    delete m;
  }

  const char *
  Status::path () const
  {
    return m->path.c_str ();
  }

  const Entry 
  Status::entry () const
  {
    if (0 == m->status)
      return Entry ();

    return Entry (m->status->entry);
  }

  const svn_wc_status_kind 
  Status::textStatus () const
  {
    return m->status->text_status;
  }

  const svn_wc_status_kind 
  Status::propStatus () const
  {
    return m->status->prop_status;
  }

  const bool 
  Status::isVersioned () const
  {
    return m->isVersioned;
  }

  const bool 
  Status::isCopied () const
  {
    return m->status->copied != 0;
  }

  const bool
  Status::isSwitched () const
  {
    return m->status->switched != 0;
  }

  const svn_wc_status_kind
  Status::reposTextStatus () const
  {
    return m->status->repos_text_status;
  }

  const svn_wc_status_kind
  Status::reposPropStatus () const
  {
    return m->status->repos_prop_status;
  }

  const bool
  Status::isLocked () const
  {
    if (m->status->repos_lock && (m->status->repos_lock->token != 0))
      return true;
    else if (m->status->entry)
      return m->status->entry->lock_token != 0;
    else
      return false;
  }

  const bool
  Status::isRepLock () const
  {
    if (m->status->entry && (m->status->entry->lock_token != 0))
      return false;
    else if (m->status->repos_lock && (m->status->repos_lock->token != 0))
      return true;
    else
      return false;
  }

  const char *
  Status::lockToken () const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->token;
    else if (m->status->entry)
      return m->status->entry->lock_token;
    else
      return "";
  }

  const char *
  Status::lockOwner () const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->owner;
    else if (m->status->entry)
      return m->status->entry->lock_owner;
    else
      return "";
  }

  const char *
  Status::lockComment () const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->comment;
    else if (m->status->entry)
      return m->status->entry->lock_comment;
    else
      return "";
  }

  const apr_time_t
  Status::lockCreationDate () const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->creation_date;
    else if (m->status->entry)
      return m->status->entry->lock_creation_date;
    else
      return 0;
  }

  Status &
  Status::operator=(const Status & src)
  {
    if (this != &src)
    {
      delete m;
      m = new Data (src.m);
    }

    return *this;
  }

  bool
  Status::isset () const
  {
    return m->path.length () > 0;
  }
}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
