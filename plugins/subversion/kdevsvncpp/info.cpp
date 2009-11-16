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

// svncpp
#include "svncpp/info.hpp"
#include "svncpp/path.hpp"
#include "svncpp/pool.hpp"

namespace svn
{
  struct Info::Data
  {
    svn_info_t * info;
    Path path;
    Pool pool;

    /** constructor (because of optional param */
    Data(const Path & path_, const svn_info_t * info_ = 0)
        : info(0), path(path_)
    {
      if (info_ != 0)
        info = svn_info_dup(info_, pool);
    }

    /** copy constructor */
    Data(const Data * src)
        : info(0), path(src->path)
    {
      if (src->info != 0)
        info = svn_info_dup(src->info, pool);
    }
  };

  Info::Info(const Path & path, const svn_info_t * info)
      : m(new Data(path, info))
  {
  }

  Info::Info(const Info & src)
      : m(new Data(src.m))
  {
  }

  Info::~Info()
  {
    delete m;
  }


  Info &
  Info::operator = (const Info & src)
  {
    if (this != &src)
    {
      delete m;
      m = new Data(src.m);
    }

    return *this;
  }

  svn_node_kind_t
  Info::kind() const
  {
    if (0 == m->info)
      return svn_node_none;
    else
      return m->info->kind;
  }

  bool
  Info::isValid() const
  {
    return m->info != 0;
  }


  const char *
  Info::url() const
  {
    if (0 == m->info)
      return 0;
    else
      return m->info->URL;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
