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

// stl
#include "kdevsvncpp/string_wrapper.hpp"

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

    Data(const char * path_, const svn_wc_status2_t * status_)
        : status(0), path("")
    {
      if (path_ != 0)
        path = path_;

      if (status_ != 0)
      {
        status = svn_wc_dup_status2(
                   const_cast<svn_wc_status2_t *>(status_), pool);
        isVersioned = status_->text_status > svn_wc_status_unversioned;
      }
    }

    Data(const Data * src)
        : status(0), path(src->path)
    {
      if (src->status != 0)
      {
        status = svn_wc_dup_status2(src->status, pool);

        switch (status->text_status)
        {
        case svn_wc_status_none:
        case svn_wc_status_unversioned:
        case svn_wc_status_ignored:
        case svn_wc_status_obstructed:
          isVersioned = false;
          break;

        default:
          isVersioned = true;
        }
      }
    }

  };

  Status::Status(const char * path, const svn_wc_status2_t * status)
      : m(new Data(path, status))
  {
  }

  Status::Status(const Status & src)
      : m(new Data(src.m))
  {
  }

  Status::~Status()
  {
    delete m;
  }

  const char *
  Status::path() const
  {
    return m->path.c_str();
  }

  const Entry
  Status::entry() const
  {
    if (0 == m->status)
      return Entry();

    return Entry(m->status->entry);
  }

  svn_wc_status_kind
  Status::textStatus() const
  {
    return m->status->text_status;
  }

  svn_wc_status_kind
  Status::propStatus() const
  {
    return m->status->prop_status;
  }

  bool
  Status::isVersioned() const
  {
    return m->isVersioned;
  }

  bool
  Status::isCopied() const
  {
    return m->status->copied != 0;
  }

  bool
  Status::isSwitched() const
  {
    return m->status->switched != 0;
  }

  svn_wc_status_kind
  Status::reposTextStatus() const
  {
    return m->status->repos_text_status;
  }

  svn_wc_status_kind
  Status::reposPropStatus() const
  {
    return m->status->repos_prop_status;
  }

  bool
  Status::isLocked() const
  {
    if (m->status->repos_lock && (m->status->repos_lock->token != 0))
      return true;
    else if (m->status->entry)
      return m->status->entry->lock_token != 0;
    else
      return false;
  }

  bool
  Status::isRepLock() const
  {
    if (m->status->entry && (m->status->entry->lock_token != 0))
      return false;
    else if (m->status->repos_lock && (m->status->repos_lock->token != 0))
      return true;
    else
      return false;
  }

  const char *
  Status::lockToken() const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->token;
    else if (m->status->entry)
      return m->status->entry->lock_token;
    else
      return "";
  }

  const char *
  Status::lockOwner() const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->owner;
    else if (m->status->entry)
      return m->status->entry->lock_owner;
    else
      return "";
  }

  const char *
  Status::lockComment() const
  {
    if (m->status->repos_lock && m->status->repos_lock->token != 0)
      return m->status->repos_lock->comment;
    else if (m->status->entry)
      return m->status->entry->lock_comment;
    else
      return "";
  }

  apr_time_t
  Status::lockCreationDate() const
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
      m = new Data(src.m);
    }

    return *this;
  }

  bool
  Status::isSet() const
  {
    return m->path.length() > 0;
  }
}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
