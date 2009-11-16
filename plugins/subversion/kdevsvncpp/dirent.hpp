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

#ifndef _SVNCPP_DIRENT_HPP_
#define _SVNCPP_DIRENT_HPP_

// subversion api
#include "svn_client.h"

namespace svn
{
  class DirEntry
  {
  public:
    /**
     * default constructor
     */
    DirEntry();

    /**
     * constructor for existing @a svn_dirent_t entries
     */
    DirEntry(const char * name, svn_dirent_t * dirEntry);

    /**
     * copy constructor
     */
    DirEntry(const DirEntry & src);

    /**
     * destructor
     */
    ~DirEntry();

    /**
     * assignment operator
     */
    DirEntry &
    operator = (const DirEntry &);

    const char *
    name() const;

    svn_node_kind_t
    kind() const;

    svn_filesize_t
    size() const;

    bool
    hasProps() const;

    svn_revnum_t
    createdRev() const;

    apr_time_t
    time() const;

    const char *
    lastAuthor() const;

  private:
    struct Data;
    Data * m;
  };
}
#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
