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
#ifndef _SVNCPP_INFO_HPP_
#define _SVNCPP_INFO_HPP_

// subversion api
#include "svn_client.h"

namespace svn
{
  // forward declarations
  class Path;

  /**
   * C++ API for Subversion.
   * This class wraps around @a svn_info_t.
   */
  class Info
  {
  public:
    /**
     * default constructor. if @a src is set,
     * copy its contents.
     *
     * If @a src is not set (=0) this will be
     * a non-versioned entry. This can be checked
     * later with @a isValid ().
     *
     * @param src another entry to copy from
     */
    Info(const Path & path, const svn_info_t * src = 0);

    /**
     * copy constructor
     */
    Info(const Info & src);

    /**
     * destructor
     */
    virtual ~Info();

    /**
     * assignment operator
     */
    Info &
    operator = (const Info &);

    /**
     * returns whether this is a valid=versioned
     * entry.
     *
     * @return is entry valid
     * @retval true valid entry
     * @retval false invalid or unversioned entry
     */
    bool isValid() const;

    /** @return entry's name */
    const Path &
    path() const;

    /** @return base revision */
    svn_revnum_t
    revision() const;

    /** @return url in repository */
    const char *
    url() const;

    /** @return canonical repository url */
    const char *
    repos() const;

    /** @return repository uuid */
    const char *
    uuid() const;

    /** @return node kind (file, dir, ...) */
    svn_node_kind_t
    kind() const;

    svn_revnum_t
    lastChangedRev() const;

    apr_time_t
    lastChangedDate() const;

    const char *
    lastChangedAuthoer() const;

    /** @todo MORE ENTRIES FROM @ref svn_info_to IF NEEDED */

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
